#ifndef UNREAL_WRAPPERS_PROP_TRAITS_H
#define UNREAL_WRAPPERS_PROP_TRAITS_H

namespace unrealsdk::unreal {

/**
 * @brief Traits class describing a UProperty.
 *
 * @tparam T The UProperty class that's being described.
 */
template <typename T>
struct PropTraits {
    /// The value type used by the described property
    using Value = void*;

    /// The unreal class name of the described property.
    static const wchar_t* const CLASS;

    /**
     * @brief Gets the value of the described property type from the given address.
     *
     * @param prop The property being get.
     * @param addr The address to read the value from.
     * @return The property's value.
     */
    [[nodiscard]] static Value get(const T* prop, uintptr_t addr);

    /**
     * @brief Sets the value of the described property type at the given address.
     *
     * @param prop The property being set.
     * @param addr The address to write the value to.
     * @param value The property's new value.
     */
    static void set(const T* prop, uintptr_t addr, Value value);
};

/**
 * @brief Gets the value of a property off of the given object.
 *
 * @tparam T The property type.
 * @param prop The property to get the value of.
 * @param idx The fixed array index to get the value at.
 * @param base_addr The base address of the object to read the property from.
 * @return The property's value.
 */
template <typename T>
[[nodiscard]] typename PropTraits<T>::Value get_property(const T* prop, size_t idx, uintptr_t base_addr) {
    if (idx > prop->ArrayDim) {
        throw std::out_of_range("Property index out of range");
    }
    return PropTraits<T>::get(prop, base_addr + prop->Offset_Internal + (idx * prop->ElementSize));
}

/**
 * @brief Set the property object
 *
 * @tparam T The property type.
 * @param prop The property to set the value of.
 * @param idx The fixed array index to set the value at.
 * @param base_addr The base address of the object to write the property to.
 * @param value The property's new value.
 */
template <typename T>
void set_property(const T* prop, size_t idx, uintptr_t base_addr, typename PropTraits<T>::Value value) {
    if (idx > prop->ArrayDim) {
        throw std::out_of_range("Property index out of range");
    }
    return PropTraits<T>::set(prop, base_addr + prop->Offset_Internal + (idx * prop->ElementSize),
                              value);
}

};  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_PROP_TRAITS_H */
