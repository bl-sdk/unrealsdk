#ifndef UNREALSDK_SIGSCAN_H
#define UNREALSDK_SIGSCAN_H

#include "unrealsdk/pch.h"

namespace unrealsdk::memory {

/**
 * @brief Performs a sigscan.
 *
 * @tparam T The type to cast the result to.
 * @param bytes The bytes to search for.
 * @param mask The mask over the bytes to search for.
 * @param pattern_size The size of the bytes + mask.
 * @param start The address to start the search at. Defaults to the start of the exe.
 * @param size The length of the region to search. Defaults to the exe size
 * @return The found location, or nullptr.
 */
uintptr_t sigscan(const uint8_t* bytes, const uint8_t* mask, size_t pattern_size);
uintptr_t sigscan(const uint8_t* bytes,
                  const uint8_t* mask,
                  size_t pattern_size,
                  uintptr_t start,
                  size_t size);
template <typename T>
T sigscan(const uint8_t* bytes, const uint8_t* mask, size_t pattern_size) {
    return reinterpret_cast<T>(sigscan(bytes, mask, pattern_size));
}
template <typename T>
T sigscan(const uint8_t* bytes,
          const uint8_t* mask,
          size_t pattern_size,
          uintptr_t start,
          size_t size) {
    return reinterpret_cast<T>(sigscan(bytes, mask, pattern_size, start, size));
}

/**
 * @brief Detours a function.
 *
 * @tparam T The signature of the detour'd function (should be picked up automatically).
 * @param addr The address of the function.
 * @param detour_func The detour function.
 * @param original_func Pointer to store the original function.
 * @param name Name of the detour, to be used in log messages on error.
 * @return True if the detour was successfully created.
 */
bool detour(uintptr_t addr, void* detour_func, void** original_func, const std::string& name);
template <typename T>
bool detour(uintptr_t addr, T detour_func, T* original_func, const std::string& name) {
    return detour(addr, reinterpret_cast<void*>(detour_func),
                  reinterpret_cast<void**>(original_func), name);
}

/**
 * @brief Reads an assembly offset, and gets the address it points to.
 *
 * @tparam T The type to cast the result to.
 * @param address The address of the offset.
 * @return The address it points to.
 */
uintptr_t read_offset(uintptr_t address);
template <typename T>
T read_offset(uintptr_t address) {
    return reinterpret_cast<T>(read_offset(address));
}

/**
 * @brief Unlocks a region of memory for full read/write access. Intended for hex edits.
 *
 * @param start The start of the range to unlock.
 * @param size The size of the range to unlock.
 */
void unlock_range(uintptr_t start, size_t size);
inline void unlock_range(uint8_t* start, size_t size) {
    unlock_range(reinterpret_cast<uintptr_t>(start), size);
}

/**
 * @brief Helper holding the values involved in a sigscan pattern.
 *
 * @tparam n The size of the pattern, in bytes.
 */
template <size_t n>
struct Pattern {
    /// The bytes to match.
    std::array<uint8_t, n> bytes;
    /// A mask over the bytes to match. May be bit-level.
    std::array<uint8_t, n> mask;
    /// A constant offset to add to the found address.
    ptrdiff_t offset = 0;

    /**
     * @brief Construct a pattern.
     *
     * @param bytes The bytes to match.
     * @param mask The mask over the bytes to match.
     * @param offset The constant offset to add to the found address.
     * @return A sigscan pattern.
     */
    Pattern(const uint8_t (&bytes)[n], const uint8_t (&mask)[n], ptrdiff_t offset = 0)
        : bytes(bytes), mask(mask), offset(offset) {}
    Pattern(const char (&bytes)[n + 1], const char (&mask)[n + 1], ptrdiff_t offset = 0)
        : bytes(reinterpret_cast<const uint8_t*>(bytes)),
          mask(reinterpret_cast<const uint8_t*>(mask)),
          offset(offset) {
        static_assert(sizeof(uint8_t) == sizeof(char), "uint8_t is different size to char");
    }

    /**
     * @brief Constructs a pattern from a hex string, at compile time.
     * @note Spaces are ignored, all other non hex characters get converted into a wildcard.
     * @note The string must contain a whole number of bytes.
     *
     * @tparam m The size of the passed hex string - should be picked up automatically.
     * @param hex The hex string to convert.
     * @param offset The constant offset to add to the found address.
     * @return A sigscan pattern.
     */
    template <size_t m>
    consteval Pattern(const char (&hex)[m], ptrdiff_t offset = 0)
        : bytes(), mask(), offset(offset) {
        size_t idx = 0;
        bool upper_nibble = true;

        for (const auto& character : hex) {
            if (character == '\0') {
                break;
            }
            if (character == ' ') {
                continue;
            }

            uint8_t byte = 0;
            uint8_t mask_byte = 0;

            // NOLINTBEGIN(readability-magic-numbers)
            if ('0' <= character && character <= '9') {
                byte = character - '0';
                mask_byte = 0xF;
            } else if ('A' <= character && character <= 'F') {
                byte = character - 'A' + 0xA;
                mask_byte = 0xF;
            } else if ('a' <= character && character <= 'f') {
                byte = character - 'a' + 0xA;
                mask_byte = 0xF;
            }
            // NOLINTEND(readability-magic-numbers)

            if (upper_nibble) {
                this->bytes[idx] = byte << 4;
                this->mask[idx] = mask_byte << 4;

                upper_nibble = false;
            } else {
                this->bytes[idx] |= byte;
                this->mask[idx] |= mask_byte;

                idx++;
                upper_nibble = true;
            }
        }

        // Make sure we completely filled the pattern, there are no missing or extra bytes, and
        // we're not halfway through one.
        if (idx != n || !upper_nibble) {
            throw "Invalid pattern size";
        }
    }

    /**
     * @brief Performs a sigscan for this pattern.
     *
     * @tparam T The type to cast the result to.
     * @param start The address to start the search at. Defaults to the start of the exe.
     * @param size The length of the region to search. Defaults to the exe size
     * @return The found location, or nullptr.
     */
    [[nodiscard]] uintptr_t sigscan(void) const {
        auto addr = memory::sigscan(this->bytes.data(), this->mask.data(), n);
        return addr == 0 ? 0 : addr + offset;
    }
    [[nodiscard]] uintptr_t sigscan(uintptr_t start, size_t size) const {
        auto addr = memory::sigscan(this->bytes.data(), this->mask.data(), n, start, size);
        return addr == 0 ? 0 : addr + offset;
    }
    template <typename T>
    [[nodiscard]] T sigscan(void) const {
        return reinterpret_cast<T>(this->sigscan());
    }
    template <typename T>
    [[nodiscard]] T sigscan(uintptr_t start, size_t size) const {
        return reinterpret_cast<T>(this->sigscan(start, size));
    }
};

}  // namespace unrealsdk::memory

#endif /* UNREALSDK_SIGSCAN_H */
