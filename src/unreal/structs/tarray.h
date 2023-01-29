#ifndef UNREAL_STRUCTS_TARRAY_H
#define UNREAL_STRUCTS_TARRAY_H

#include "pch.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

template <class T>
struct TArray {
   public:
    T* data;
    int32_t count;
    int32_t max;

    static constexpr const auto MAX_CAPACITY = INT32_MAX;

    /**
     * @brief Gets the size of the array.
     *
     * @return The size of the array.
     */
    [[nodiscard]] size_t size(void) const { return this->count; };

    /**
     * @brief Gets the capacity of the array.
     *
     * @return The capacity of the array.
     */
    [[nodiscard]] size_t capacity(void) const { return this->max; };

    /**
     * @brief Get an element in the array.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    [[nodiscard]] U operator[](size_t idx) const {
        return this->data[idx];
    };
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    U& operator[](size_t idx) {
        return this->data[idx];
    };

    /**
     * @brief Get an element in the array, with bounds checking.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    [[nodiscard]] U at(size_t idx) const {
        if (idx > this->count) {
            throw std::out_of_range("TArray index out of range");
        }
        return this->operator[](idx);
    }

    /**
     * @brief Reserves memory to increase the capacity of this array.
     * @note A `TArray<void>` is assumed to have 1-byte elements for the purposes of this function.
     *
     * @param new_cap The new capacity, in number of elements.
     */
    void reserve(size_t new_cap);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_TARRAY_H */
