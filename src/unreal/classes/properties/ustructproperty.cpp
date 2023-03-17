#include "pch.h"

#include "unreal/cast_prop.h"
#include "unreal/classes/properties/ustructproperty.h"
#include "unreal/classes/uscriptstruct.h"
#include "unreal/prop_traits.h"
#include "unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

PropTraits<UStructProperty>::Value PropTraits<UStructProperty>::get(
    const UStructProperty* prop,
    uintptr_t addr,
    const std::shared_ptr<void>& parent) {
    auto this_struct = prop->read_field(&UStructProperty::Struct);
    return {this_struct, reinterpret_cast<void*>(addr), parent};
}

void PropTraits<UStructProperty>::set(const UStructProperty* prop,
                                      uintptr_t addr,
                                      const Value& value) {
    auto this_struct = prop->read_field(&UStructProperty::Struct);
    if (value.type != this_struct) {
        throw std::runtime_error("Struct is not an instance of " + (std::string)this_struct->Name);
    }

    for (const auto& inner : this_struct->properties()) {
        cast_prop(inner, [addr, &value]<typename T>(const T* inner) {
            for (size_t i = 0; i < inner->ArrayDim; i++) {
                set_property<T>(inner, i, addr, value.get<T>(inner, i));
            }
        });
    }
}

void PropTraits<UStructProperty>::destroy(const UStructProperty* prop, uintptr_t addr) {
    auto this_struct = prop->read_field(&UStructProperty::Struct);

    for (const auto& inner : this_struct->properties()) {
        cast_prop(inner, [addr]<typename T>(const T* inner) {
            for (size_t i = 0; i < inner->ArrayDim; i++) {
                destroy_property<T>(inner, i, addr);
            }
        });
    }
}

}  // namespace unrealsdk::unreal
