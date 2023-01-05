#ifndef SIGSCAN_H
#define SIGSCAN_H

#include "pch.h"
#include <vadefs.h>

namespace unrealsdk::sigscan {

/**
 * @brief Struct holding information about a sigscan.
 */
struct Pattern {
   public:
    const uint8_t* bytes;
    const uint8_t* mask;
    const size_t offset;
    const size_t size;

    /**
     * @brief Construct a pattern from strings.
     *
     * @tparam n The length of the strings (should be picked up automatically).
     * @param bytes The bytes to match.
     * @param mask The mask over the bytes to match.
     * @param offset The constant offset to add to the found address.
     * @return A sigscan pattern.
     */
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    template <size_t n>
    Pattern(const char (&bytes)[n], const char (&mask)[n], size_t offset = 0)
        : bytes(reinterpret_cast<const uint8_t*>(bytes)),
          mask(reinterpret_cast<const uint8_t*>(mask)),
          offset(offset),
          size(n - 1) {}

    static_assert(sizeof(uint8_t) == sizeof(char), "uint8_t is different size to char");
    // NOLINTEND(modernize-avoid-c-arrays)
};

/**
 * @brief Performs a sigscan.
 *
 * @tparam T the type to cast the result to.
 * @param start The address to start the search at.
 * @param size The length of the region to search.
 * @param pattern The pattern to search for.
 * @return The found location, or nullptr.
 */
uintptr_t scan(uintptr_t start, size_t size, const Pattern& pattern);
template <typename T>
T scan(uintptr_t start, size_t size, const Pattern& pattern) {
    return reinterpret_cast<T>(scan(start, size, pattern));
}

/**
 * @brief Reads an assembly offset, and gets the address it points to.
 *
 * @tparam T the type to cast the result to.
 * @param address The address of the offset.
 * @return The address it points to.
 */
uintptr_t read_offset(uintptr_t address);
template <typename T>
T read_offset(uintptr_t address) {
    return reinterpret_cast<T>(read_offset(address));
}

/**
 * @brief Gets the address range covered by the exe's module.
 *
 * @return A tuple of the exe start address and it's length.
 */
std::tuple<uintptr_t, size_t> get_exe_range(void);

}  // namespace unrealsdk::sigscan

#endif /* SIGSCAN_H */
