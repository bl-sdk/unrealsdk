#ifndef UNREALSDK_UNREAL_CAST_H
#define UNREALSDK_UNREAL_CAST_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/class_name.h"

// Note: this header needs to pull in all unreal classes
// Refrain from including it in other headers
#include "unrealsdk/unreal/classes/ublueprintgeneratedclass.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/properties/attribute_property.h"
#include "unrealsdk/unreal/properties/copyable_property.h"
#include "unrealsdk/unreal/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/properties/zarrayproperty.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"
#include "unrealsdk/unreal/properties/zbyteproperty.h"
#include "unrealsdk/unreal/properties/zclassproperty.h"
#include "unrealsdk/unreal/properties/zcomponentproperty.h"
#include "unrealsdk/unreal/properties/zdelegateproperty.h"
#include "unrealsdk/unreal/properties/zenumproperty.h"
#include "unrealsdk/unreal/properties/zgamedatahandleproperty.h"
#include "unrealsdk/unreal/properties/zgbxdefptrproperty.h"
#include "unrealsdk/unreal/properties/zgbxinlinestructproperty.h"
#include "unrealsdk/unreal/properties/zinterfaceproperty.h"
#include "unrealsdk/unreal/properties/zmulticastdelegateproperty.h"
#include "unrealsdk/unreal/properties/zobjectproperty.h"
#include "unrealsdk/unreal/properties/zproperty.h"
#include "unrealsdk/unreal/properties/zstrproperty.h"
#include "unrealsdk/unreal/properties/zstructproperty.h"
#include "unrealsdk/unreal/properties/ztextproperty.h"
#include "unrealsdk/unreal/properties/zweakobjectproperty.h"
#include "unrealsdk/unreal/structs/ffield.h"

namespace unrealsdk::unreal {

/**
 * @brief A tuple of all unreal classes.
 * @note Intended for to be used to iterate over all types using recursive templates.
 */
using all_unreal_classes = std::tuple<  // formatting
    FField,
    UBlueprintGeneratedClass,
    UClass,
    UConst,
    UEnum,
    UField,
    UFunction,
    UObject,
    UScriptStruct,
    UStruct,
    ZArrayProperty,
    ZBoolProperty,
    ZByteAttributeProperty,
    ZByteProperty,
    ZClassProperty,
    ZComponentProperty,
    ZDelegateProperty,
    ZDoubleProperty,
    ZEnumProperty,
    ZFloatAttributeProperty,
    ZFloatProperty,
    ZGameDataHandleProperty,
    ZGbxDefPtrProperty,
    ZGbxInlineStructProperty,
    ZInt16Property,
    ZInt64Property,
    ZInt8Property,
    ZIntAttributeProperty,
    ZIntProperty,
    ZInterfaceProperty,
    ZLazyObjectProperty,
    ZMulticastDelegateProperty,
    ZNameProperty,
    ZObjectProperty,
    ZProperty,
    ZSoftClassProperty,
    ZSoftObjectProperty,
    ZStrProperty,
    ZStructProperty,
    ZTextProperty,
    ZUInt16Property,
    ZUInt32Property,
    ZUInt64Property,
    ZWeakObjectProperty
    /* formatting */>;

namespace {

#ifdef __clang__
// Clangd doesn't seem to realize we use this below
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

/**
 * @brief Default fallback used when casting fails, throws a runtime error.
 *
 * @param obj The object which failed to cast.
 */
template <typename T>
inline void default_cast_fallback(const T* obj) {
    throw std::runtime_error("Unknown object type " + obj->Class()->Name());
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

/**
 * @brief Implementation of cast - kept private as it has less friendly args + template args.
 *
 * @tparam InputType The type of the input object.
 * @tparam ClassType The type of the working class object.
 * @tparam Function The type of the callback function.
 * @tparam Fallback The type of the fallback function.
 * @tparam include_input_type True if the input type is a valid output type.
 * @tparam check_inherited_types True if to check inherited types if the first pass fails to match.
 * @tparam ClassTuple A tuple of all classes to check.
 * @tparam i The index of the tuple currently being compared against.
 * @param obj The object being cast.
 * @param working_class The class currently being matched.
 * @param func The callback function.
 * @param fallback The fallback function.
 */
template <typename InputType,
          typename ClassType,
          typename Function,
          typename Fallback,
          bool include_input_type,
          bool check_inherited_types,
          typename ClassTuple,
          size_t i>
void cast_impl(InputType* obj,
               ClassType* working_class,
               const Function& func,
               const Fallback& fallback) {
    // If out of elements
    if constexpr (i >= std::tuple_size_v<ClassTuple>) {
        // But we're supposed to check inherited types, and we have a super field
        if constexpr (check_inherited_types) {
            if (working_class->SuperField() != nullptr) {
                // Jump back to the start of the tuple, but use the super field
                return cast_impl<InputType, ClassType, Function, Fallback, include_input_type,
                                 check_inherited_types, ClassTuple, 0>(
                    obj, working_class->SuperField(), func, fallback);
            }
        }

        // Call the fallback
        return fallback(obj);

    } else {
        // If we still have elements to check
        using cls = std::tuple_element_t<i, ClassTuple>;

        // If this class inherits from the input type
        if constexpr (std::is_base_of_v<std::remove_const_t<InputType>, cls>
                      && (include_input_type
                          || !std::is_same_v<std::remove_const_t<InputType>, cls>)) {
            // If the class name matches
            if (working_class->Name() == cls_fname<cls>()) {
                // Run the callback
                if constexpr (std::is_const_v<InputType>) {
                    return func.template operator()<cls>(reinterpret_cast<const cls*>(obj));
                } else {
                    return func.template operator()<cls>(reinterpret_cast<cls*>(obj));
                }
            }
        }

        // Try the next element
        return cast_impl<InputType, ClassType, Function, Fallback, include_input_type,
                         check_inherited_types, ClassTuple, i + 1>(obj, working_class, func,
                                                                   fallback);
    }
}

}  // namespace

/**
 * @brief Type used to store the options for a cast.
 *
 * @tparam include_input_type_ If a cast to the same type as the input variable is considered valid.
 *                             False by default, as a cast usually implies you want a more derived
 *                             type, and ignoring the input type means the callback does not need to
 *                             special case it (e.g. in case of missing specializations).
 * @tparam check_inherited_types_ If to fall back to checking inherited types if the first pass
 *                                fails to find a match. False by default
 * @tparam class_tuple_ The tuple of possible classes. By default contains all known unreal classes,
 *                      can be redefined to only check a subset.
 */
template <bool include_input_type_ = false,
          bool check_inherited_types_ = false,
          typename class_tuple_ = all_unreal_classes>
// NOLINTBEGIN(readability-identifier-naming)
struct cast_options {
    /// If a cast to the same type as the input variable is considered valid.
    ///
    /// False by default, as a cast usually implies you want a more derived type, and ignoring the
    /// input type means the callback does not need to special case it (e.g. in case of missing
    /// specializations).
    static const constexpr bool include_input_type_v = include_input_type_;

    /// If to fall back to checking inherited types if the first pass fails to find a match.
    ///
    /// False by default, as this would imply the object is of an unknown type, which we won't know
    /// how to handle.
    static const constexpr bool check_inherited_types_v = check_inherited_types_;

    /// A tuple of classes to check
    ///
    /// By default contains all known unreal classes, can be redefined to only check a subset.
    using class_tuple_t = class_tuple_;
    // NOLINTEND(readability-identifier-naming)

    /// Helpers to create new options, replacing a single field

    template <bool new_include_input>
    using with_input = cast_options<new_include_input, check_inherited_types_, class_tuple_>;

    template <bool new_include_inherited>
    using with_inherited = cast_options<include_input_type_, new_include_inherited, class_tuple_>;

    template <typename new_classes>
    using with_classes = cast_options<include_input_type_, check_inherited_types_, new_classes>;
};

/**
 * @brief Works out what type an object is at runtime, and calls a templated callback function with
 *        it's actual type.
 * @note By default, only casts to classes deriving from the type of the input object (not including
 *       itself). Can be customized using `cast_options`.
 *
 * @tparam Options The options to use for this cast.
 * @tparam InputType The type of the input object, the least derived type which may be cast to.
 * @tparam Function The type of the callback function. The signature should be `void(T* obj)` (or
 *                  `void(const T* obj)` if `InputType` is const), where `T` is a template arg which
 *                  will derive from (or be equal to) `std::remove_const_t<InputType>`.
 * @tparam Fallback The fallback function's type, only templated for auto deduction. The signature
                    should be `void(InputType* obj)` or `void(const InputType* obj)`.
 * @param obj The object to cast.
 * @param func The templated callback function to call.
 * @param fallback A function to call when casting fails. Defaults to throwing a runtime error.
 */
template <typename Options = cast_options<>,
          typename InputType,
          typename Function,
          typename Fallback = std::function<void(InputType*)>>
void cast(InputType* obj,
          const Function& func,
          const Fallback& fallback = default_cast_fallback<InputType>)
    requires(std::is_base_of_v<UObject, InputType> || std::is_base_of_v<FField, InputType>)
{
    if (obj == nullptr) {
        throw std::invalid_argument("Tried to cast null object!");
    }

    auto working_cls = obj->Class();
    using ClassType = std::remove_cvref_t<decltype(*working_cls->SuperField())>;
    return cast_impl<InputType, ClassType, Function, Fallback, Options::include_input_type_v,
                     Options::check_inherited_types_v, typename Options::class_tuple_t, 0>(
        obj, working_cls, func, fallback);
}

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CAST_H */
