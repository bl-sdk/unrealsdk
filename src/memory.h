#ifndef SIGSCAN_H
#define SIGSCAN_H

#include "pch.h"
#include <vadefs.h>

namespace unrealsdk::memory {

/**
 * @brief Struct holding information about a sigscan pattern.
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
    template <size_t n>
    Pattern(const char (&bytes)[n], const char (&mask)[n], size_t offset = 0)
        : bytes(reinterpret_cast<const uint8_t*>(bytes)),
          mask(reinterpret_cast<const uint8_t*>(mask)),
          offset(offset),
          size(n - 1) {}

    static_assert(sizeof(uint8_t) == sizeof(char), "uint8_t is different size to char");
};

/**
 * @brief Performs a sigscan.
 *
 * @tparam T The type to cast the result to.
 * @param pattern The pattern to search for.
 * @param start The address to start the search at. Defaults to the start of the exe.
 * @param size The length of the region to search. Defaults to the exe size
 * @return The found location, or nullptr.
 */
uintptr_t sigscan(const Pattern& pattern);
template <typename T>
T sigscan(const Pattern& pattern) {
    return reinterpret_cast<T>(sigscan(pattern));
}
uintptr_t sigscan(const Pattern& pattern, uintptr_t start, size_t size);
template <typename T>
T sigscan(const Pattern& pattern, uintptr_t start, size_t size) {
    return reinterpret_cast<T>(sigscan(pattern, start, size));
}

/**
 * @brief Helper which sigscans for a function and detours it.
 *
 * @tparam T The signature of the detour'd function (should be picked up automatically).
 * @param pattern The pattern to search for.
 * @param detour The detour function.
 * @param original Pointer to store the original function.
 * @param name Name of the detour, to be used in log messages.
 * @param start The address to start the search at. Defaults to the start of the exe.
 * @param size The length of the region to search. Defaults to the exe size
 * @return True if the detour was successfully created.
 */
bool sigscan_and_detour(const Pattern& pattern,
                        void* detour,
                        void** original,
                        const std::string& name);
template <typename T>
bool sigscan_and_detour(const Pattern& pattern, T detour, T* original, const std::string& name) {
    return sigscan_and_detour(pattern, reinterpret_cast<void*>(detour),
                              reinterpret_cast<void**>(original), name);
}
bool sigscan_and_detour(const Pattern& pattern,
                        void* detour,
                        void** original,
                        const std::string& name,
                        uintptr_t start,
                        size_t size);
template <typename T>
bool sigscan_and_detour(const Pattern& pattern,
                        T detour,
                        T* original,
                        const std::string& name,
                        uintptr_t start,
                        size_t size) {
    return sigscan_and_detour(pattern, reinterpret_cast<void*>(detour),
                              reinterpret_cast<void**>(original), name, start, size);
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
 * @brief Unlocks a region of memeory for full read/write access. Intended for hex edits.
 *
 * @param start The start of the range to unlock.
 * @param size The size of the range to unlock.
 */
void unlock_range(uintptr_t start, size_t size);
inline void unlock_range(uint8_t* start, size_t size) {
    unlock_range(reinterpret_cast<uintptr_t>(start), size);
}

}  // namespace unrealsdk::memory

#endif /* SIGSCAN_H */
