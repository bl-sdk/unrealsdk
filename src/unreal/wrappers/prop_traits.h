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
    [[nodiscard]] static Value get(T* prop, uintptr_t addr);

    /**
     * @brief Sets the value of the described property type at the given address.
     *
     * @param prop The property being set.
     * @param addr The address to write the value to.
     * @param value The property's new value.
     */
    static void set(T* prop, uintptr_t addr, Value value);
};

};  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_PROP_TRAITS_H */
