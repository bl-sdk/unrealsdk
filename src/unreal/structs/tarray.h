#ifndef UNREAL_STRUCTS_TARRAY_H
#define UNREAL_STRUCTS_TARRAY_H

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

    /**
     * @brief Get the size of the array.
     *
     * @return The size of the array.
     */
    [[nodiscard]] size_t size(void) const { return this->count; };

    /**
     * @brief Get an element in the array.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    [[nodiscard]] T operator[](size_t idx) const { return this->data[idx]; };
    T& operator[](size_t idx) { return this->data[idx]; };

    /**
     * @brief Get an element in the array, with bounds checking.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    [[nodiscard]] T at(size_t idx) const {
        if (idx > this->count) {
            throw std::out_of_range("TArray index out of range");
        }
        return this->operator[](idx);
    }
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_TARRAY_H */
