#ifndef UNREAL_CLASSES_UOBJECT_FUNCS_H
#define UNREAL_CLASSES_UOBJECT_FUNCS_H

#include "pch.h"

#include "unreal/classes/uobject.h"
#include "unreal/classes/ustruct.h"
#include "unreal/classes/ustruct_funcs.h"
#include "unreal/wrappers/bound_function.h"
#include "unreal/wrappers/prop_traits.h"

namespace unrealsdk::unreal {

class UFunction;

template <>
[[nodiscard]] typename PropTraits<UFunction>::Value UObject::get<UFunction>(const FName& name,
                                                                            size_t idx) const;
template <>
[[nodiscard]] typename PropTraits<UFunction>::Value UObject::get<UFunction>(const UFunction* prop,
                                                                            size_t idx) const;

template <typename T>
[[nodiscard]] typename PropTraits<T>::Value UObject::get(const FName& name, size_t idx) const {
    return this->get<T>(this->Class->find_and_validate<T>(name), idx);
}
template <typename T>
[[nodiscard]] typename PropTraits<T>::Value UObject::get(const T* prop, size_t idx) const {
    return get_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this));
}

template <typename T>
void UObject::set(const FName& name, size_t idx, const typename PropTraits<T>::Value& value) {
    this->set<T>(this->Class->find_and_validate<T>(name), idx, value);
}
template <typename T>
void UObject::set(const T* prop, size_t idx, const typename PropTraits<T>::Value& value) {
    set_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this), value);
}

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UOBJECT_FUNCS_H */
