#include "pch.h"

#include "unreal/classes/properties/ustructproperty.h"
#include "unreal/classes/uscriptstruct.h"
#include "unreal/wrappers/wrapped_struct.h"


namespace unrealsdk::unreal {

PropTraits<UStructProperty>::Value PropTraits<UStructProperty>::get(UStructProperty* prop,
                                                                    uintptr_t addr) {
    auto this_struct = prop->read_field(&UStructProperty::Struct);
    return {this_struct, reinterpret_cast<void*>(addr)};
}

void PropTraits<UStructProperty>::set(UStructProperty* prop, uintptr_t addr, Value value) {
    auto this_struct = prop->read_field(&UStructProperty::Struct);
    if (value.type != this_struct) {
        throw std::runtime_error("Struct is not an instance of " + (std::string)this_struct->Name);
    }

    memcpy(reinterpret_cast<void*>(addr), value.base, prop->ElementSize);
}

}  // namespace unrealsdk::unreal
