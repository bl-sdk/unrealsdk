#ifndef UNREALSDK_MULTI_SIGSCAN_H
#define UNREALSDK_MULTI_SIGSCAN_H

#include "unrealsdk/pch.h"
#include "unrealsdk/memory.h"

namespace unrealsdk::memory {

/**
 * @brief Helper type for multi-sigscans patterns. Will get the result written to it.
 * @note This is a reference type, it holds references to the pattern it's constructed from.
 */
struct MultiPattern {
    const uint8_t* bytes;
    const uint8_t* mask;
    size_t pattern_size;
    ptrdiff_t offset;

    size_t start_offset = 0;
    std::atomic<uintptr_t> result = 0;

    /**
     * @brief Construct a multi-pattern from a regular one.
     *
     * @tparam n The size of the pattern - should be picked up automatically.
     * @param pattern The pattern to base this one off of.
     */
    template <size_t n>
    consteval MultiPattern(const Pattern<n>& pattern)
        : bytes(pattern.bytes.data()),
          mask(pattern.mask.data()),
          pattern_size(n),
          offset(pattern.offset) {}

    /**
     * @brief Gets the resolved address.
     *
     * @return The address.
     */
    uintptr_t addr(void) const { return this->result.load(); }
};

namespace {

/**
 * @brief Preprocesses patterns for a multi-sigscan.
 *
 * @tparam n The size of the array - should be picked up automatically.
 * @param patterns The array of patterns to analyse.
 * @return A pair of the common byte to scan for, and the amount to overscan by.
 */
template <size_t n>
std::pair<uint8_t, size_t> multi_sigscan_preprocess(const std::array<MultiPattern*, n>& patterns) {
    // TODO: Sure would be nice if this was consteval
    // To be able to write the result back, the patterns can't be constexpr, which complicates it

    // So there's two ideas behind the multi-sigscan, both of which we need to gather some data for:
    // - We find a byte which exists in all patterns, then use std::find to do an optimized search
    //   for it, and only do the slow O(nm) sigscan anchored on each match.
    // - We split the game's memory into chunks, and check one per thread. This needs a little
    //   overscanning in case a pattern happens to fall on a boundary.

    // NOLINTBEGIN(readability-magic-numbers)
    std::array<uint8_t, 256> byte_counts{};
    std::array<uint8_t, 256> patterns_with_byte{};
    for (MultiPattern* pattern : patterns) {
        std::array<uint64_t, 4> seen_bytes{};

        for (size_t i = 0; i < pattern->pattern_size; i++) {
            // Must not be masked out
            if (pattern->mask[i] != 0xFF) {
                continue;
            }
            uint8_t byte = pattern->bytes[i];
            // saturating add
            byte_counts.at(byte) = (uint8_t)std::min(255, byte_counts.at(byte) + 1);
            seen_bytes.at(byte / 64) |= ((uint64_t)1 << (byte % 64));
        }

        for (size_t i = 0; i < 256; i++) {
            if (seen_bytes.at(i / 64) & ((uint64_t)1 << (i % 64))) {
                patterns_with_byte.at(i) = (uint8_t)std::min(255, patterns_with_byte.at(i) + 1);
            }
        }
    }
    // NOLINTEND(readability-magic-numbers)

#undef UNREALSDK_MULTI_SIGSCAN_LOGGING
#ifdef UNREALSDK_MULTI_SIGSCAN_LOGGING
    LOG(MISC, "Multi-sigscan stats across {} patterns:", n);
    LOG(MISC, "Total uses of byte", n);
    LOG(MISC, "    00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F");
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 0; i < 256; i += 16) {
        LOG(MISC,
            "{:02X} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} "
            "{:3}",
            i, byte_counts.at(i + 0), byte_counts.at(i + 1), byte_counts.at(i + 2),
            byte_counts.at(i + 3), byte_counts.at(i + 4), byte_counts.at(i + 5),
            byte_counts.at(i + 6), byte_counts.at(i + 7), byte_counts.at(i + 8),
            byte_counts.at(i + 9), byte_counts.at(i + 10), byte_counts.at(i + 11),
            byte_counts.at(i + 12), byte_counts.at(i + 13), byte_counts.at(i + 14),
            byte_counts.at(i + 15));
    }
    LOG(MISC, "Num patterns with byte:", n);
    LOG(MISC, "    00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F");
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 0; i < 256; i += 16) {
        LOG(MISC,
            "{:02X} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} {:3} "
            "{:3}",
            i, patterns_with_byte.at(i + 0), patterns_with_byte.at(i + 1),
            patterns_with_byte.at(i + 2), patterns_with_byte.at(i + 3),
            patterns_with_byte.at(i + 4), patterns_with_byte.at(i + 5),
            patterns_with_byte.at(i + 6), patterns_with_byte.at(i + 7),
            patterns_with_byte.at(i + 8), patterns_with_byte.at(i + 9),
            patterns_with_byte.at(i + 10), patterns_with_byte.at(i + 11),
            patterns_with_byte.at(i + 12), patterns_with_byte.at(i + 13),
            patterns_with_byte.at(i + 14), patterns_with_byte.at(i + 15));
    }
#endif

    // Ignore some common bytes
    // NOLINTBEGIN(readability-magic-numbers)
    patterns_with_byte[0x00] = 0;
    patterns_with_byte[0x24] = 0;
    patterns_with_byte[0x48] = 0;
    patterns_with_byte[0xFF] = 0;
    // NOLINTEND(readability-magic-numbers)

    // Of all bytes which appear in every pattern, pick the one with the lowest total count, as a
    // rough heuristic for how common we expect the byte to be in the exe.
    uint8_t byte{};
    uint32_t found_byte_count = std::numeric_limits<uint32_t>::max();
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 0; i < 256; i++) {
        if (patterns_with_byte.at(i) < n) {
            continue;
        }
        if (byte_counts.at(i) < found_byte_count) {
            found_byte_count = byte_counts.at(i);
            byte = (uint8_t)i;
        }
    }

    if (found_byte_count == std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("couldn't find common byte between all patterns");
    }

    size_t overscan = 0;
    for (MultiPattern* pattern : patterns) {
        for (size_t i = 0; i < pattern->pattern_size; i++) {
            // NOLINTNEXTLINE(readability-magic-numbers)
            if (pattern->mask[i] != 0xFF || pattern->bytes[i] != byte) {
                continue;
            }

            // Setting this probably also messes with the consteval
            pattern->start_offset = i;
            overscan = std::max(overscan, pattern->pattern_size - i - 1);
            break;
        }
    }

    return {byte, overscan};
}

/**
 * @brief Performs a multi-sigscan over a given memory region.
 *
 * @tparam n The size of the patterns array - should be picked up automatically.
 * @param pos The scan starting position.
 * @param end The scan ending position (exclusive).
 * @param patterns The array of patterns to scan for.
 * @param byte The common anchor byte to search for.
 */
template <size_t n>
void multi_sigscan_thread(const uint8_t* pos,
                          const uint8_t* end,
                          const std::array<MultiPattern*, n>& patterns,
                          uint8_t byte) {
    do {
        const auto* const match = std::find(pos, end, byte);
        if (match == end) {
            return;
        }

#ifdef UNREALSDK_MULTI_SIGSCAN_LOGGING
        size_t pos_idx = 0;
#endif
        for (MultiPattern* pattern : patterns) {
            const uint8_t* sig_start = match - pattern->start_offset;

            const uint8_t* mask_pos = pattern->mask;
            if (std::ranges::equal(sig_start, sig_start + pattern->pattern_size, pattern->bytes,
                                   pattern->bytes + pattern->pattern_size, std::ranges::equal_to{},
                                   [&mask_pos](uint8_t byte) { return byte & (*mask_pos++); })) {
                auto result = reinterpret_cast<uintptr_t>(sig_start) + pattern->offset;

                // We have a match. Try swap into the result, if we don't already have one, and it's
                // smaller than the old result.
                auto old_result = pattern->result.load(std::memory_order::memory_order_relaxed);
                auto new_result = old_result != 0 && old_result < result ? old_result : result;

                while (!pattern->result.compare_exchange_weak(
                    old_result, new_result, std::memory_order::memory_order_seq_cst,
                    std::memory_order::memory_order_relaxed)) {
                    new_result = old_result != 0 && old_result < result ? old_result : result;
                }

#ifdef UNREALSDK_MULTI_SIGSCAN_LOGGING
                LOG(MISC, "Multi-sigscan match in pattern {} at {:p}, sig at {:p}, result now {:p}",
                    pos_idx, reinterpret_cast<const void*>(match),
                    reinterpret_cast<const void*>(sig_start),
                    reinterpret_cast<const void*>(pattern->result.load()));
#endif
            }

#ifdef UNREALSDK_MULTI_SIGSCAN_LOGGING
            pos_idx++;
#endif
        }

        pos = match + 1;
    } while (true);
}

}  // namespace

/**
 * @brief Sigscan for all the given patterns, at the same time, and write their results back.
 *
 * @param patterns_arg Pointers to the patterns to scan for.
 */
template <typename... T>
void multi_sigscan(T*... patterns_arg) {
    std::array<MultiPattern*, sizeof...(T)> patterns{{patterns_arg...}};

    auto [byte, overscan] = multi_sigscan_preprocess(patterns);

    size_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        // May return 0 if not supported.
        // Curently steam hardware survey says 6 and 8 cores are ~25% each, so lets go with the
        // better of the two.
        // NOLINTNEXTLINE(readability-magic-numbers)
        num_threads = 8;
    }
    // Assuming we hit memory throughput on all our threads, leave one for the game's initalization
    // This is entirely theoretical, I have not measured
    num_threads--;

    // Number pulled from thin air
    constexpr size_t min_size_per_thread = 4 * 1024 * 1024;
    auto [start, size] = get_exe_range();
    size_t size_per_thread = std::max(size / num_threads, min_size_per_thread);

    LOG(MISC, "Multi-sigscan common byte: {:X}, overscan {:X}, per thread {:X}", byte, overscan,
        size_per_thread);

    std::vector<std::thread> threads;
    for (size_t start_offset = 0; start_offset < size; start_offset += size_per_thread) {
        auto* thread_pos = reinterpret_cast<uint8_t*>(start + start_offset);
        auto* thread_end = thread_pos + size_per_thread + overscan;
        thread_end = std::min(thread_end, reinterpret_cast<uint8_t*>(start) + size);
        threads.emplace_back([thread_pos, thread_end, &patterns, byte]() {
            multi_sigscan_thread(thread_pos, thread_end, patterns, byte);
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

}  // namespace unrealsdk::memory

#endif /* UNREALSDK_MULTI_SIGSCAN_H */
