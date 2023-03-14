#ifndef UNREAL_CLASSES_USTRUCT_FUNCS_H
#define UNREAL_CLASSES_USTRUCT_FUNCS_H

#include "pch.h"

#include "unreal/class_name.h"
#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

template <typename T>
[[nodiscard]] T* UStruct::find_prop_and_validate(const FName& name) const {
    return validate_type<T>(this->find_prop(name));
}

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_USTRUCT_FUNCS_H */
