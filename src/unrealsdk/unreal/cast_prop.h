#ifndef UNREAL_CAST_PROP_H
#define UNREAL_CAST_PROP_H

#include "pch.h"

#include "unreal/class_name.h"
#include "unreal/classes/properties/copyable_property.h"
#include "unreal/classes/properties/uarrayproperty.h"
#include "unreal/classes/properties/uboolproperty.h"
#include "unreal/classes/properties/uclassproperty.h"
#include "unreal/classes/properties/uinterfaceproperty.h"
#include "unreal/classes/properties/uobjectproperty.h"
#include "unreal/classes/properties/ustrproperty.h"
#include "unreal/classes/properties/ustructproperty.h"
#include "unreal/classes/ustruct.h"
#include "unreal/prop_traits.h"

namespace unrealsdk::unreal {

/// A tuple of all known property types.
using all_known_properties = std::tuple<
    // Copyable properties
    UInt8Property,
    UInt16Property,
    UIntProperty,
    UInt64Property,
    UByteProperty,
    UUInt16Property,
    UUInt32Property,
    UUInt64Property,
    UFloatProperty,
    UDoubleProperty,
    UNameProperty,
    // Other properties
    UArrayProperty,
    UBoolProperty,
    UClassProperty,
    UInterfaceProperty,
    UObjectProperty,
    UStrProperty,
    UStructProperty>;

/**
 * @brief Helper which turns a const pointer type back into it's base type.
 *
 * @tparam T The const pointer type to convert.
 * @return The base type.
 */
template <typename T>
using remove_const_pointer_t = std::remove_const_t<std::remove_pointer<T>>;

/**
 * @brief Works out what type a property is at runtime, and calls a templated callback function with
 *        it's correct type.
 * @note Callback should have the signature `template<typename T> void callback(const T* prop)`
 *
 * @tparam F The type of the callback function. Should be picked up automatically.
 * @tparam i Index to start searching through properties at. Should always call with 0 (the
 * default).
 * @param prop The property to cast.
 * @param func The callback function to call
 */
template <typename F, int i = 0>
void cast_prop(const UProperty* prop, const F& func) {
    if constexpr (i >= std::tuple_size_v<all_known_properties>) {
        throw std::runtime_error("Unknown property type " + (std::string)prop->Class->Name);
    } else {
        using prop_type = std::tuple_element_t<i, all_known_properties>;
        static const auto PROP_CLS_NAME = cls_fname<prop_type>();
        if (PROP_CLS_NAME == prop->Class->Name) {
            func.template operator()<prop_type>(reinterpret_cast<const prop_type*>(prop));
        } else {
            cast_prop<F, i + 1>(prop, func);
        }
    }
}

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CAST_PROP_H */
