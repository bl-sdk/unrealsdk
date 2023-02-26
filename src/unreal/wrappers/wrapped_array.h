#ifndef UNREAL_WRAPPERS_WRAPPED_ARRAY_H
#define UNREAL_WRAPPERS_WRAPPED_ARRAY_H

#include "unreal/classes/uclass.h"
#include "unreal/classes/uproperty.h"
#include "unreal/structs/fname.h"
#include "unreal/structs/tarray.h"
#include "unreal/wrappers/prop_traits.h"
#include "utils.h"

namespace unrealsdk::unreal {

class WrappedArray {
   public:
    const UProperty* type;
    TArray<void>* base;

    /**
     * @brief Gets the size of the array.
     *
     * @return The size of the array.
     */
    [[nodiscard]] size_t size(void) const;

    /**
     * @brief Reserves memory to change the capacity of this array.
     * @note Can be used to shrink the array.
     *
     * @param new_cap The new capacity, in number of elements.
     */
    void reserve(size_t new_cap) const;

    /**
     * @brief Resizes the array.
     * @note Does not necessarily impact reserved capacity, just current amount of elements.
     *
     * @param new_size The new size, in number of elements.
     */
    void resize(size_t new_size) const;

   private:
    /**
     * @brief Type and bound check an access to this array.
     *
     * @tparam T The expected property type
     * @param idx The index being accessed.
     */
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<UProperty, T>>>
    void validate_access(size_t idx) const {
        const auto EXPECTED_CLASS = FName{PropTraits<T>::CLASS};
        auto property_class = this->type->Class->Name;
        if (property_class != EXPECTED_CLASS) {
            throw std::invalid_argument("WrappedArray property was of invalid type "
                                        + (std::string)property_class);
        }

        if (idx > this->base->count) {
            throw std::out_of_range("WrappedArray index out of range");
        }
    }

   public:
    /**
     * @brief Gets an element in the array, with bounds and type checking.
     *
     * @tparam T The expected property type
     * @param idx The index to get.
     * @return The item at that index.
     */
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<UProperty, T>>>
    [[nodiscard]] typename PropTraits<T>::Value get_at(size_t idx) const {
        this->validate_access<T>(idx);
        return PropTraits<T>::get(
            reinterpret_cast<const T*>(this->type),
            reinterpret_cast<uintptr_t>(this->base->data) + (idx * this->type->ElementSize));
    }

    /**
     * @brief Sets an element in the array, with bounds and type checking.
     *
     * @tparam T The expected property type
     * @param idx The index to get.
     * @return The item at that index.
     */
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<UProperty, T>>>
    void set_at(size_t idx, typename PropTraits<T>::Value value) {
        this->validate_access<T>(idx);
        return PropTraits<T>::set(
            reinterpret_cast<const T*>(this->type),
            reinterpret_cast<uintptr_t>(this->base->data) + (idx * this->type->ElementSize), value);
    }
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_WRAPPED_ARRAY_H */
