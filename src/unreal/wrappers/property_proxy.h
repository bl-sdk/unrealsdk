#ifndef UNREAL_WRAPPERS_PROPERTY_PROXY_H
#define UNREAL_WRAPPERS_PROPERTY_PROXY_H

#include "pch.h"
#include <stdexcept>

#include "unreal/class_name.h"
#include "unreal/prop_traits.h"
#include "unrealsdk.h"

namespace unrealsdk::unreal {

class UProperty;

class PropertyProxy {
   public:
    UProperty* prop;

   private:
    std::shared_ptr<void> value;

   public:
    /**
     * @brief Constructs a new property proxy.
     *
     * @param prop The property.
     */
    PropertyProxy(UProperty* prop);

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
                               reinterpret_cast<uintptr_t>(this->value.get()), this->value);
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
        auto prop = validate_type<T>(this->prop);

        if (!this->has_value()) {
            auto deleter = [prop](void* data) {
                for (size_t i = 0; i < prop->ArrayDim; i++) {
                    destroy_property<T>(prop, i, reinterpret_cast<uintptr_t>(data));
                }
                unrealsdk::u_free(data);
            };

            this->value = {unrealsdk::u_malloc(prop->ElementSize * prop->ArrayDim), deleter};
        }

        set_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this->value.get()), value);
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
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_PROPERTY_PROXY_H */
