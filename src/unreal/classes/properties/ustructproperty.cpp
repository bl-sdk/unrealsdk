#include "pch.h"

#include "unreal/classes/properties/ustructproperty.h"
#include "unreal/classes/uscriptstruct.h"
#include "unreal/wrappers/wrapped_struct.h"


namespace unrealsdk::unreal {

PropTraits<UStructProperty>::Value PropTraits<UStructProperty>::get(UStructProperty* prop,
                                                                    uintptr_t addr) {
    return {prop->Struct, reinterpret_cast<void*>(addr)};
}

void PropTraits<UStructProperty>::set(UStructProperty* prop, uintptr_t addr, Value value) {
    if (value.type != prop->Struct) {
        throw std::runtime_error("Struct is not an instance of " + (std::string)prop->Struct->Name);
    }

    memcpy(reinterpret_cast<void*>(addr), value.base, prop->ElementSize);
}

}  // namespace unrealsdk::unreal
