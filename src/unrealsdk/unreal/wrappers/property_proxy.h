#ifndef UNREALSDK_UNREAL_WRAPPERS_PROPERTY_PROXY_H
#define UNREALSDK_UNREAL_WRAPPERS_PROPERTY_PROXY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

class UProperty;

class PropertyProxy {
   public:
    UProperty* prop;

   private:
    void* value;
    bool been_set;

    /**
     * @brief Get the address access to the value should use.
     *
     * @return The address access to the value should use.
     */
    [[nodiscard]] uintptr_t get_value_addr(void) const;

   public:
    /**
     * @brief Constructs a new property proxy.
     *
     * @param prop The property.
     * @param other The other property proxy to construct this one from.
     */
    PropertyProxy(UProperty* prop);
    PropertyProxy(const PropertyProxy& other);
    PropertyProxy(PropertyProxy&& other) noexcept;

    /**
     * @brief Assigns to the property proxy.
     * @note Only allowed if of the same type.
     *
     * @param other The other property proxy to assign this one from.
     * @return A reference to this property proxy.
     */
    PropertyProxy& operator=(const PropertyProxy& other);
    PropertyProxy& operator=(PropertyProxy&& other) noexcept;

    /**
     * @brief Destroy the property proxy.
     */
    ~PropertyProxy();

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
        if (!this->been_set) {
            throw std::runtime_error(
                "Tried to get value of a property proxy which is yet to be set!");
        }

        return get_property<T>(validate_type<T>(this->prop), idx, this->get_value_addr());
    }

    /**
     * @brief Sets the stored value.
     *
     * @tparam T The property type.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @param value The new stored value.
     */
    template <typename T>
    void set(const typename PropTraits<T>::Value& value) {
        this->set<T>(0, value);
    }
    template <typename T>
    void set(size_t idx, const typename PropTraits<T>::Value& value) {
        if (this->prop == nullptr) {
            throw std::runtime_error("Property does not exist!");
        }

        set_property<T>(validate_type<T>(this->prop), idx, this->get_value_addr(), value);
        this->been_set = true;
    }

    /**
     * @brief Destroys the stored value.
     */
    void destroy(void);

    /**
     * @brief Copies the stored property to another address.
     *
     * @param addr The address to copy to.
     */
    void copy_to(uintptr_t addr);

    /**
     * @brief Copies the value of another address to the stored property.
     *
     * @param addr The address to copy from.
     */
    void copy_from(uintptr_t addr);
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_WRAPPERS_PROPERTY_PROXY_H */
