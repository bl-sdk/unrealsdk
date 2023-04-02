#ifndef UNREAL_WRAPPERS_WRAPPED_ARRAY_H
#define UNREAL_WRAPPERS_WRAPPED_ARRAY_H

#include "pch.h"

#include "unreal/class_name.h"
#include "unreal/classes/uclass.h"
#include "unreal/classes/uproperty.h"
#include "unreal/prop_traits.h"
#include "unreal/structs/fname.h"
#include "unreal/structs/tarray.h"
#include "utils.h"

namespace unrealsdk::unreal {

class WrappedArray {
   public:
    const UProperty* type;
    std::shared_ptr<TArray<void>> base;

    /**
     * @brief Constructs a new wrapped array.
     *
     * @param type The type of the array elements.
     * @param base The base address of the array.
     * @param parent The parent pointer this array was retrieved from, used to copy ownership.
     */
    WrappedArray(const UProperty* type,
                 TArray<void>* base,
                 const std::shared_ptr<void>& parent = {nullptr});

    /**
     * @brief Gets the size of the array.
     *
     * @return The size of the array.
     */
    [[nodiscard]] size_t size(void) const;

    /**
     * @brief Reserves memory to change the capacity of this array.
     * @note Can be used to shrink the capacity - caller must ensure any removed entries are already
     *       destroyed, so they don't leak.
     *
     * @param new_cap The new capacity, in number of elements.
     */
    void reserve(size_t new_cap) const;

    /**
     * @brief Resizes the array.
     * @note Does not necessarily impact reserved capacity, just current amount of elements.
     * @note Caller must ensure any removed entries are already destroyed, so they don't leak.
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
    template <typename T>
    void validate_access(size_t idx) const {
        auto property_class = this->type->Class->Name;
        if (property_class != cls_fname<T>()) {
            throw std::invalid_argument("WrappedArray property was of invalid type "
                                        + (std::string)property_class);
        }

        if (idx > (size_t)this->base->count) {
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
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get_at(size_t idx) const {
        this->validate_access<T>(idx);
        return get_property<T>(reinterpret_cast<const T*>(this->type), idx,
                               reinterpret_cast<uintptr_t>(this->base->data), this->base);
    }

    /**
     * @brief Sets an element in the array, with bounds and type checking.
     *
     * @tparam T The expected property type
     * @param idx The index to set.
     * @param value The new value to set.
     */
    template <typename T>
    void set_at(size_t idx, const typename PropTraits<T>::Value& value) {
        this->validate_access<T>(idx);
        set_property<T>(reinterpret_cast<const T*>(this->type), idx,
                        reinterpret_cast<uintptr_t>(this->base->data), value);
    }

    /**
     * @brief Destroyes n element in the array, with bounds and type checking.
     *
     * @tparam T The expected property type
     * @param idx The index to destory.
     */
    template <typename T>
    void destroy_at(size_t idx) {
        this->validate_access<T>(idx);
        destroy_property<T>(reinterpret_cast<const T*>(this->type), idx,
                            reinterpret_cast<uintptr_t>(this->base->data));
    }
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_WRAPPED_ARRAY_H */
