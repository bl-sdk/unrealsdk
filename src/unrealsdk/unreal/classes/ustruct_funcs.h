#ifndef UNREALSDK_UNREAL_CLASSES_USTRUCT_FUNCS_H
#define UNREALSDK_UNREAL_CLASSES_USTRUCT_FUNCS_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

template <typename T>
[[nodiscard]] T* UStruct::find_prop_and_validate(const FName& name) const {
    return validate_type<T>(this->find_prop(name));
}

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_USTRUCT_FUNCS_H */
