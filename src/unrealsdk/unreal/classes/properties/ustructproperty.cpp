#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

[[nodiscard]] UScriptStruct* UStructProperty::get_inner_struct(void) const {
    return this->read_field(&UStructProperty::Struct);
}

PropTraits<UStructProperty>::Value PropTraits<UStructProperty>::get(
    const UStructProperty* prop,
    uintptr_t addr,
    const UnrealPointer<void>& parent) {
    auto this_struct = prop->get_inner_struct();
    return {this_struct, reinterpret_cast<void*>(addr), parent};
}

void PropTraits<UStructProperty>::set(const UStructProperty* prop,
                                      uintptr_t addr,
                                      const Value& value) {
    auto this_struct = prop->get_inner_struct();
    if (value.type != this_struct) {
        throw std::runtime_error("Struct is not an instance of " + (std::string)this_struct->Name);
    }
    copy_struct(addr, value);
}

void PropTraits<UStructProperty>::destroy(const UStructProperty* prop, uintptr_t addr) {
    destroy_struct(prop->get_inner_struct(), addr);
}

}  // namespace unrealsdk::unreal
