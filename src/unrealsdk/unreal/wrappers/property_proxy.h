#ifndef UNREALSDK_UNREAL_WRAPPERS_PROPERTY_PROXY_H
#define UNREALSDK_UNREAL_WRAPPERS_PROPERTY_PROXY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

class UProperty;

struct PropertyProxy {
   public:
    UProperty* prop;
    UnrealPointer<void> ptr;

    /**
     * @brief Constructs a new property proxy.
     *
     * @param prop The property.
     * @param other The other property proxy to construct this one from.
     */
    PropertyProxy(UProperty* prop);
    PropertyProxy(const PropertyProxy& other);
    PropertyProxy(PropertyProxy&& other) noexcept = default;

    /**
     * @brief Assigns to the property proxy.
     * @note Only allowed if of the same type.
     *
     * @param other The other property proxy to assign this one from.
     * @return A reference to this property proxy.
     */
    PropertyProxy& operator=(const PropertyProxy& other);
    PropertyProxy& operator=(PropertyProxy&& other) noexcept = default;

    /**
     * @brief Destroy the property proxy.
     */
    ~PropertyProxy() = default;

    /**
     * @brief Checks if we have a value stored.
     *
     * @return True if we have a value stored.
     */
    [[nodiscard]] bool has_value(void) const;

    /**
     * @brief Gets the stored value.
     *
     * @tparam T The type of the property.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @return The property's value.
     */
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(size_t idx = 0) const {
        if (!this->has_value()) {
            throw std::runtime_error(
                "Tried to get value of a property proxy which is yet to be set!");
        }

        return get_property<T>(validate_type<T>(this->prop), idx,
                               reinterpret_cast<uintptr_t>(this->ptr.get()), this->ptr);
    }

    /**
     * @brief Sets the stored value.
     *
     * @tparam T The property type.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @param new_value The new stored value.
     */
    template <typename T>
    void set(const typename PropTraits<T>::Value& new_value) {
        this->set<T>(0, new_value);
    }
    template <typename T>
    void set(size_t idx, const typename PropTraits<T>::Value& new_value) {
        if (this->prop == nullptr) {
            throw std::runtime_error("Cannot set null property!");
        }

        if (!this->has_value()) {
            this->ptr = UnrealPointer<void>{this->prop};
        }
        set_property<T>(validate_type<T>(this->prop), idx,
                        reinterpret_cast<uintptr_t>(this->ptr.get()), new_value);
    }

    /**
     * @brief Destroys the stored value.
     */
    void destroy(void);

    // The following functions aren't too useful for user code, but sdk internals still find them
    // handy to have available.

    /**
     * @brief Copies the stored property to another address.
     *
     * @param addr The address to copy to.
     */
    void copy_to(uintptr_t addr) const;

    /**
     * @brief Copies the value of another address to the stored property.
     *
     * @param addr The address to copy from.
     */
    void copy_from(uintptr_t addr);
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_WRAPPERS_PROPERTY_PROXY_H */
