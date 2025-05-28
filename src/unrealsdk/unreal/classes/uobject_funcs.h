#ifndef UNREALSDK_UNREAL_CLASSES_UOBJECT_FUNCS_H
#define UNREALSDK_UNREAL_CLASSES_UOBJECT_FUNCS_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/classes/ustruct_funcs.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"

namespace unrealsdk::unreal {

class UFunction;

template <>
[[nodiscard]] BoundFunction UObject::get<UFunction, BoundFunction>(const FName& name,
                                                                   size_t idx) const;
template <>
[[nodiscard]] BoundFunction UObject::get<UFunction, BoundFunction>(const UFunction* prop,
                                                                   size_t idx) const;

template <typename T, typename V>
[[nodiscard]] V UObject::get(const FName& name, size_t idx) const {
    return this->get<T>(this->Class()->find_prop_and_validate<T>(name), idx);
}
template <typename T, typename V>
[[nodiscard]] V UObject::get(const T* prop, size_t idx) const {
    return get_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this));
}

template <typename T>
void UObject::set(const FName& name, size_t idx, const typename PropTraits<T>::Value& value) {
    this->set<T>(this->Class()->find_prop_and_validate<T>(name), idx, value);
}
template <typename T>
void UObject::set(const T* prop, size_t idx, const typename PropTraits<T>::Value& value) {
    set_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this), value);
}

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UOBJECT_FUNCS_H */
