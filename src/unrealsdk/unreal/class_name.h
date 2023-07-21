#ifndef UNREALSDK_UNREAL_CLASS_NAME_H
#define UNREALSDK_UNREAL_CLASS_NAME_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

class UBlueprintGeneratedClass;
class UFunction;
class UScriptStruct;
class UProperty;

/**
 * @brief Gets the unreal class name of the templated type.
 *
 * @tparam T The type to get the name of.
 * @return The class' fname.
 */
template <typename T>
[[nodiscard]] FName cls_fname(void) {
    static FName name{PropTraits<T>::CLASS};
    return name;
}

template <>
[[nodiscard]] FName cls_fname<UObject>(void);
template <>
[[nodiscard]] FName cls_fname<UField>(void);
template <>
[[nodiscard]] FName cls_fname<UStruct>(void);
template <>
[[nodiscard]] FName cls_fname<UClass>(void);
template <>
[[nodiscard]] FName cls_fname<UBlueprintGeneratedClass>(void);
template <>
[[nodiscard]] FName cls_fname<UFunction>(void);
template <>
[[nodiscard]] FName cls_fname<UScriptStruct>(void);
template <>
[[nodiscard]] FName cls_fname<UProperty>(void);

/**
 * @brief Throws an invalid argument exception if an object is not of the expected type
 * @note Uses an exact type match, not if it's an instance.
 *
 * @tparam T Type type the object is expected to be.
 * @param obj Pointer to the object.
 */
template <typename T>
void throw_on_wrong_type(const UObject* obj) {
    if (obj == nullptr) {
        throw std::invalid_argument("Tried to validate type of null object!");
    }
    static const auto expected_cls_name = cls_fname<T>();
    auto cls_name = obj->Class->Name;
    if (cls_name != expected_cls_name) {
        throw std::invalid_argument("Object was of unexpected type " + (std::string)cls_name);
    }
}

/**
 * @brief Validates that an object is of the expected type, and returns it cast to that type.
 * @note Uses an exact type match, not if it's an instance.
 *
 * @tparam T Type type the object is expected to be.
 * @param obj Pointer to the object.
 * @return The object cast to the expected type.
 */
template <typename T>
T* validate_type(UObject* obj) {
    throw_on_wrong_type<T>(obj);
    return reinterpret_cast<T*>(obj);
}
template <typename T>
const T* validate_type(const UObject* obj) {
    throw_on_wrong_type<T>(obj);
    return reinterpret_cast<const T*>(obj);
}

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASS_NAME_H */
