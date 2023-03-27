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
     * @brief Sets the return value.
     *
     * @tparam T The property type.
     * @param value The new return value.
     */
    template <typename T>
    void set(const typename PropTraits<T>::Value& value) {
        this->set<T>(0, value);
    }
    template <typename T>
    void set(size_t idx, const typename PropTraits<T>::Value& value) {
        if (!this->has_value()) {
            // Make sure we have the correct type before we allocate anything
            validate_type<T>(this->prop);

            auto deleter = [this](void* data) {
                for (size_t i = 0; i < this->prop->ArrayDim; i++) {
                    destroy_property<T>(this->prop, i, reinterpret_cast<uintptr_t>(data));
                }
                unrealsdk::u_free(data);
            };

            value = {unrealsdk::u_malloc(this->prop->ElementSize * this->prop->ArrayDim), deleter};
        }

        set_property(prop, idx, reinterpret_cast<uintptr_t>(this->value.get()), value);
    }
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_PROPERTY_PROXY_H */
