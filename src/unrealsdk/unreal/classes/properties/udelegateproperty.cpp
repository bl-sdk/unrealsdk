#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/udelegateproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

UFunction* UDelegateProperty::get_signature(void) const {
    return this->read_field(&UDelegateProperty::Signature);
}

PropTraits<UDelegateProperty>::Value PropTraits<UDelegateProperty>::get(
    const UDelegateProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    return reinterpret_cast<FScriptDelegate*>(addr)->as_function();
}

void PropTraits<UDelegateProperty>::set(const UDelegateProperty* prop,
                                        uintptr_t addr,
                                        const Value& value) {
    FScriptDelegate::validate_signature(value, prop->get_signature());
    reinterpret_cast<FScriptDelegate*>(addr)->bind(value);
}

}  // namespace unrealsdk::unreal
