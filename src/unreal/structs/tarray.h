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
     * @brief Reserves memory to increase the capacity of this array.
     *
     * @param new_cap The new capacity, in number of elements.
     * @param element_size The size of each element.
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    void reserve(size_t new_cap) {
        this->reserve(new_cap, sizeof(U));
    }
    void reserve(size_t new_cap, size_t element_size);

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

#pragma region Iterator
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = U;
        using pointer = U*;
        using reference = U;

       private:
        TArray<U>* arr;
        size_t idx{};

       public:
        Iterator(TArray<U>* arr) : arr(arr) {}

        reference operator*() const { return (*this->arr)[this->idx]; };

        Iterator& operator++() {
            ++this->idx;
            // Use `arr == nullptr` as the end condition, so we behave a little better if the array
            //  grows during iteration - we can't guarentee control over this iterator as well as
            //  the others
            if (this->idx >= arr->count) {
                arr = nullptr;
            }
            return *this;
        };
        Iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& rhs) const {
            if (this->arr == nullptr && rhs.arr == nullptr) {
                return true;
            }
            return this->arr == rhs.arr && this->idx == rhs.idx;
        }
        bool operator!=(const Iterator& rhs) const { return !(*this == rhs); };
    };

    /**
     * @brief Gets an iterator to the start of this array.
     * @note Will continue to work if the array changes size during iteration. Shrinking beyond the
     *       element currently pointed at is undefined behaviour.
     *
     * @return The iterator.
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    [[nodiscard]] Iterator<U> begin(void) {
        return {this->count == 0 ? nullptr : this};
    }

    /**
     * @brief Gets an iterator to the end of this array.
     *
     * @return The iterator.
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    [[nodiscard]] static Iterator<U> end(void) {
        return {nullptr};
    }
#pragma endregion
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_TARRAY_H */
