#ifndef UNREAL_WRAPPERS_PROP_UTILS_H
#define UNREAL_WRAPPERS_PROP_UTILS_H

#include "pch.h"

#include "unreal/classes/uclass.h"
#include "unreal/classes/uobject.h"
#include "unreal/classes/uproperty.h"
#include "unreal/classes/ustruct.h"
#include "unreal/wrappers/prop_traits.h"
#include "unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

#pragma region Helpers
namespace {

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
[[nodiscard]] typename PropTraits<T>::Value get_property(T* prop, size_t idx, uintptr_t base_addr) {
    if (idx > prop->ArrayDim) {
        throw std::out_of_range("Property index out of range");
    }
    return PropTraits<T>::get(base_addr + prop->Offset_Internal + (idx * prop->ElementSize));
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
void set_property(T* prop, size_t idx, uintptr_t base_addr, typename PropTraits<T>::Value value) {
    if (idx > prop->ArrayDim) {
        throw std::out_of_range("Property index out of range");
    }
    return PropTraits<T>::set(base_addr + prop->Offset_Internal + (idx * prop->ElementSize), value);
}

}  // namespace
#pragma endregion

#pragma region Forward Declaration Implementations

template <typename T>
[[nodiscard]] T* UStruct::find_and_validate(const FName& name) const {
    const auto CLASS = FName{PropTraits<T>::CLASS};

    for (auto prop : *this) {
        if (prop->Name != name) {
            continue;
        }

        auto cls_fname = reinterpret_cast<UObject*>(prop->Class)->Name;
        if (cls_fname != CLASS) {
            throw std::invalid_argument("Property was of invalid type " + (std::string)cls_fname);
        }
        return reinterpret_cast<T*>(prop);
    }

    throw std::invalid_argument("Couldn't find property " + (std::string)name);
}

template <typename T>
[[nodiscard]] typename PropTraits<T>::Value UObject::get(const FName& name, size_t idx) {
    return get_property<T>(this->Class->find_and_validate<T>(name), idx,
                           reinterpret_cast<uintptr_t>(this));
}

template <typename T>
void UObject::set(const FName& name, size_t idx, typename PropTraits<T>::Value value) {
    set_property<T>(this->Class->find_and_validate<T>(name), idx, reinterpret_cast<uintptr_t>(this),
                    value);
}

template <typename T>
[[nodiscard]] typename PropTraits<T>::Value WrappedStruct::get(const FName& name, size_t idx) {
    return get_property<T>(this->type->find_and_validate<T>(name), idx,
                           reinterpret_cast<uintptr_t>(this->base));
}

template <typename T>
void WrappedStruct::set(const FName& name, size_t idx, typename PropTraits<T>::Value value) {
    set_property<T>(this->type->find_and_validate<T>(name), idx,
                    reinterpret_cast<uintptr_t>(this->base), value);
}

#pragma endregion

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_PROP_UTILS_H */
