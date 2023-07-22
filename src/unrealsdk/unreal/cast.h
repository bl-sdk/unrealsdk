#ifndef UNREALSDK_UNREAL_CAST_H
#define UNREALSDK_UNREAL_CAST_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/class_name.h"

// Note: this header needs to pull in all unreal classes
// Refrain from including it in other headers
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uenumproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/ublueprintgeneratedclass.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

/**
 * @brief A tuple of all unreal classes.
 * @note Intended for to be used to iterate over all types using recursive templates.
 */
using all_unreal_classes = std::tuple<  //
    UArrayProperty,
    UBlueprintGeneratedClass,
    UBoolProperty,
    UByteProperty,
    UClass,
    UClassProperty,
    UConst,
    UDoubleProperty,
    UEnum,
    UEnumProperty,
    UField,
    UFloatProperty,
    UFunction,
    UInt16Property,
    UInt64Property,
    UInt8Property,
    UInterfaceProperty,
    UIntProperty,
    UNameProperty,
    UObject,
    UObjectProperty,
    UProperty,
    UScriptStruct,
    UStrProperty,
    UStruct,
    UStructProperty,
    UUInt16Property,
    UUInt32Property,
    UUInt64Property>;

/**
 * @brief Specifies that the input object's class is a valid output type.
 */
using include_input_class = std::bool_constant<true>;

/**
 * @brief Specifies that the input object's class is not a valid output type.
 */
using ignore_input_class = std::bool_constant<false>;

namespace {

/**
 * @brief Default fallback used when casting fails, throws a runtime error.
 *
 * @param obj The object which failed to cast.
 */
inline void default_cast_fallback(const UObject* obj) {
    throw std::runtime_error("Unknown object type " + (std::string)obj->Class->Name);
}

}  // namespace

/**
 * @brief Works out what type an object is at runtime, and calls a templated callback function with
 *        it's actual type.
 * @note By default, only casts to classes deriving from the type of the input object (not including
 *       itself). Call with `cast<include_input_class, UObject>(...)` to cast to any known type.
 *
 * @tparam InputClassHandling One of `include_input_class` or `ignore_input_class`, to specify if
 *                            the input class is a valid output type.
 * @tparam InputType The type of the input object, the least derived type which may be cast to.
 * @tparam ClassTuple A tuple of all classes to search through. Defaults to all unreal classes.
 * @tparam Function The type of the callback function. The signature should be `void(const T* obj)`,
 *                  where `T` is a template arg which will derive from (or be equal to) `InputType`.
 * @tparam Fallback The fallback function's type, only templated for auto deduction. The signature
                    should be `void(const InputType* obj)`.
 * @tparam i Index to start searching through classes at. Should always call with 0 (the default).
 * @param obj The object to cast.
 * @param func The templated callback function to call.
 * @param fallback A function to call when casting fails. Defaults to throwing a runtime error.
 */
template <typename InputClassHandling = ignore_input_class,
          typename InputType,
          typename ClassTuple = all_unreal_classes,
          typename Function,
          typename Fallback = std::function<void(const InputType*)>,
          size_t i = 0,
          typename = std::enable_if_t<std::is_base_of_v<UObject, InputType>>,
          typename = std::enable_if_t<
              std::disjunction_v<std::is_same<InputClassHandling, include_input_class>,
                                 std::is_same<InputClassHandling, ignore_input_class>>>>
void cast(const InputType* obj,
          const Function& func,
          const Fallback& fallback = default_cast_fallback) {
    if constexpr (i == 0) {
        if (obj == nullptr) {
            throw std::invalid_argument("Tried to cast null object!");
        }
    }

    if constexpr (i >= std::tuple_size_v<ClassTuple>) {
        return fallback(obj);
    } else {
        using cls = std::tuple_element_t<i, ClassTuple>;

        if constexpr (std::is_base_of_v<InputType, cls>
                      && (InputClassHandling::value || !std::is_same_v<InputType, cls>)) {
            if (obj->Class->Name == cls_fname<cls>()) {
                return func.template operator()<cls>(reinterpret_cast<const cls*>(obj));
            }
        }

        return cast<InputClassHandling, InputType, ClassTuple, Function, Fallback, i + 1>(obj, func,
                                                                                          fallback);
    }
}

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CAST_H */
