#ifndef UNREAL_CLASSES_USTRUCT_FUNCS_H
#define UNREAL_CLASSES_USTRUCT_FUNCS_H

#include "pch.h"

#include "unreal/classes/uclass.h"
#include "unreal/classes/ustruct.h"
#include "unreal/structs/fname.h"

namespace unrealsdk::unreal {

template <typename T>
[[nodiscard]] T* UStruct::find_and_validate(const FName& name) const {
    const auto EXPECTED_CLASS = FName{PropTraits<T>::CLASS};

    auto field = this->find(name);
    auto field_class = field->Class->Name;
    if (field_class != EXPECTED_CLASS) {
        throw std::invalid_argument("Property was of invalid type " + (std::string)field_class);
    }
    return reinterpret_cast<T*>(field);
}

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_USTRUCT_FUNCS_H */
