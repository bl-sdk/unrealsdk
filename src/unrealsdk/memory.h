#ifndef UNREALSDK_MEMORY_H
#define UNREALSDK_MEMORY_H

#include "unrealsdk/pch.h"

namespace unrealsdk::memory {

template <size_t n>
struct Pattern;

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
 * @tparam n The size of the sigscan pattern (should be picked up automatically).
 * @param addr The address of the function.
 * @param pattern A sigscan pattern matching the function to detour.
 * @param detour_func The detour function.
 * @param original_func Pointer to store the original function.
 * @param name Name of the detour, to be used in log messages on error.
 * @return True if the detour was successfully created.
 */
bool detour(uintptr_t addr, void* detour_func, void** original_func, std::string_view name);
template <typename T>
bool detour(uintptr_t addr, T detour_func, T* original_func, std::string_view name) {
    return detour(addr, reinterpret_cast<void*>(detour_func),
                  reinterpret_cast<void**>(original_func), name);
}
template <size_t n>
bool detour(const Pattern<n>& pattern,
            void* detour_func,
            void** original_func,
            std::string_view name) {
    // Use sigscan nullable since the base detour function will warn about a null address
    return detour(pattern.sigscan_nullable(), detour_func, original_func, name);
}
template <typename T, size_t n>
bool detour(const Pattern<n>& pattern, T detour_func, T* original_func, std::string_view name) {
    return detour(pattern.sigscan_nullable(), reinterpret_cast<void*>(detour_func),
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

   private:
    /**
     * @brief Converts a hex character to it's nibble and a mask.
     *
     * @param character The character.
     * @return A pair of the nibble and it's mask.
     */
    consteval std::pair<uint8_t, uint8_t> char_to_nibble_and_mask(char character) {
        // NOLINTBEGIN(readability-magic-numbers)
        if ('0' <= character && character <= '9') {
            return {(uint8_t)(character - '0'), (uint8_t)0xF};
        }
        if ('A' <= character && character <= 'F') {
            return {(uint8_t)(character - 'A' + 0xA), (uint8_t)0xF};
        }
        if ('a' <= character && character <= 'f') {
            return {(uint8_t)(character - 'a' + 0xA), (uint8_t)0xF};
        }
        return {(uint8_t)0, (uint8_t)0};
        // NOLINTEND(readability-magic-numbers)
    }

   public:
    /**
     * @brief Constructs a pattern from a hex string, at compile time.
     * @note An opening curly bracket sets the offset - only the first instance is used.
     * @note Spaces and closing curly brackets are ignored.
     * @note All other characters are considered wildcards.
     * @note The string must contain a whole number of bytes. Nibble wildcards are allowed.
     *
     * @tparam m The size of the passed hex string - should be picked up automatically.
     * @param hex The hex string to convert.
     * @param offset The constant offset to add to the found address.
     * @return A sigscan pattern.
     */
    template <size_t m>
    consteval Pattern(const char (&hex)[m],
                      ptrdiff_t offset = std::numeric_limits<ptrdiff_t>::max())
        : bytes(), mask(), offset(offset) {
        size_t idx = 0;
        bool upper_nibble = true;

        for (const auto& character : hex) {
            if (character == '\0') {
                break;
            }
            if (character == ' ' || character == '}') {
                continue;
            }
            if (character == '{') {
                if (!upper_nibble) {
                    throw std::logic_error("Cannot start pattern offset halfway through a byte");
                }
                if (this->offset == std::numeric_limits<ptrdiff_t>::max()) {
                    this->offset = idx;
                }
                continue;
            }

            auto [nibble, nibble_mask] = char_to_nibble_and_mask(character);
            if (upper_nibble) {
                this->bytes[idx] = nibble << 4;
                this->mask[idx] = nibble_mask << 4;

                upper_nibble = false;
            } else {
                this->bytes[idx] |= nibble;
                this->mask[idx] |= nibble_mask;

                idx++;
                upper_nibble = true;
            }
        }

        // Make sure we completely filled the pattern, there are no missing or extra bytes, and
        // we're not halfway through one.
        if (idx != n || !upper_nibble) {
            throw std::logic_error("Invalid pattern size");
        }

        if (this->offset == std::numeric_limits<ptrdiff_t>::max()) {
            this->offset = 0;
        }
    }

    /**
     * @brief Performs a sigscan for this pattern across the main executable.
     * @note When not found, `sigscan` throws, while `sigscan_nullable` returns 0.
     *
     * @tparam T The type to cast the result to.
     * @param name The name of this pattern, to use in error messages.
     * @return The found location, or 0.
     */
    [[nodiscard]] uintptr_t sigscan(std::string_view name) const {
        auto addr = memory::sigscan(this->bytes.data(), this->mask.data(), n);
        if (addr == 0) {
            // Make sure to log something on error, even if calling code doesn't catch it
            LOG(ERROR, "Sigscan for {} failed!", name);
            throw std::runtime_error("sigscan failed");
        }
        return addr == 0 ? 0 : addr + offset;
    }
    template <typename T>
    [[nodiscard]] T sigscan(std::string_view name) const {
        return reinterpret_cast<T>(this->sigscan(name));
    }
    [[nodiscard]] uintptr_t sigscan_nullable(void) const {
        auto addr = memory::sigscan(this->bytes.data(), this->mask.data(), n);
        return addr == 0 ? 0 : addr + offset;
    }
    template <typename T>
    [[nodiscard]] T sigscan_nullable(void) const {
        return reinterpret_cast<T>(this->sigscan_nullable());
    }
};

/**
 * @brief Gets the address range covered by the exe's module.
 *
 * @return A tuple of the exe start address and it's length.
 */
std::pair<uintptr_t, size_t> get_exe_range(void);

}  // namespace unrealsdk::memory

#endif /* UNREALSDK_MEMORY_H */
