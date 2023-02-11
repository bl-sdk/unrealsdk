#ifndef UNREAL_CLASSES_UOBJECT_FUNCS_H
#define UNREAL_CLASSES_UOBJECT_FUNCS_H

#include "pch.h"

#include "unreal/classes/uobject.h"
#include "unreal/classes/ustruct.h"
#include "unreal/classes/ustruct_funcs.h"
#include "unreal/wrappers/prop_traits.h"

namespace unrealsdk::unreal {

template <typename T>
[[nodiscard]] typename PropTraits<T>::Value UObject::get(const FName& name, size_t idx) const {
    return get_property<T>(this->Class->find_and_validate<T>(name), idx,
                           reinterpret_cast<uintptr_t>(this));
}

template <typename T>
void UObject::set(const FName& name, size_t idx, typename PropTraits<T>::Value value) {
    set_property<T>(this->Class->find_and_validate<T>(name), idx, reinterpret_cast<uintptr_t>(this),
                    value);
}

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UOBJECT_FUNCS_H */
