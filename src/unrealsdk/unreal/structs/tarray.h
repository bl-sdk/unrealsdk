#ifndef UNREALSDK_UNREAL_STRUCTS_TARRAY_H
#define UNREALSDK_UNREAL_STRUCTS_TARRAY_H

#include "unrealsdk/pch.h"

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

    static constexpr const auto MAX_CAPACITY = std::numeric_limits<decltype(count)>::max();

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
     * @brief Changes the capacity of this array, reserving new memory as needed.
     * @note Can be used to shrink the capacity.
     * @note Caller must ensure any removed entries are already destroyed, so they don't leak, and
     *       that any added entries' values are appropriately initialized.
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

   private:
    static const constexpr auto MIN_GROW = 4;
    static const constexpr auto GROW_MULTIPLIER = 3;
    static const constexpr auto GROW_DIVIDER = 8;
    static const constexpr auto GROW_CONST = 16;

   public:
    /**
     * @brief Resizes the array.
     * @note Caller must ensure any removed entries are already destroyed, so they don't leak, and
     *       that any added entries' values are appropriately initialized.
     *
     * @param new_size The new size, in number of elements.
     * @param element_size The size of each element.
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    void resize(size_t new_size) {
        this->resize(new_size, sizeof(U));
    }
    void resize(size_t new_size, size_t element_size) {
        if ((size_t)this->max < new_size) {
            // Vaguely copying the standard UE grow with slack logic
            auto new_capacity =
                (new_size <= MIN_GROW)
                    ? MIN_GROW
                    : new_size + (GROW_MULTIPLIER * new_size / GROW_DIVIDER) + GROW_CONST;

            this->reserve(new_capacity, element_size);
        }

        this->count = (decltype(count))new_size;
    }

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
    }
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    U& operator[](size_t idx) {
        return this->data[idx];
    }

    /**
     * @brief Get an element in the array, with bounds checking.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    [[nodiscard]] U at(size_t idx) const {
        if (idx >= (size_t)this->count) {
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
        const TArray<U>* arr;
        size_t idx{};

       public:
        Iterator(void) : arr(nullptr) {}
        Iterator(const TArray<U>* arr) : arr(arr) {}

        reference operator*() const { return (*this->arr)[this->idx]; };

        Iterator& operator++() {
            ++this->idx;
            // Use `arr == nullptr` as the end condition, so we behave a little better if the array
            // grows during iteration
            if (this->idx >= (size_t)arr->count) {
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
    [[nodiscard]] Iterator<U> begin(void) const {
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
        return {};
    }
#pragma endregion
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_TARRAY_H */
