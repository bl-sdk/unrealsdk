#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/umulticastdelegateproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_multicast_delegate.h"

namespace unrealsdk::unreal {

UFunction* UMulticastDelegateProperty::get_signature(void) const {
    return this->read_field(&UMulticastDelegateProperty::Signature);
}

PropTraits<UMulticastDelegateProperty>::Value PropTraits<UMulticastDelegateProperty>::get(
    const UMulticastDelegateProperty* prop,
    uintptr_t addr,
    const UnrealPointer<void>& parent) {
    return {prop->get_signature(), reinterpret_cast<TArray<FScriptDelegate>*>(addr), parent};
}

void PropTraits<UMulticastDelegateProperty>::set(const UMulticastDelegateProperty* prop,
                                                 uintptr_t addr,
                                                 const Value& value) {
    if (value.signature != prop->get_signature()) {
        throw std::runtime_error("Multicast delegate signature doesn't match existing signature of "
                                 + (std::string)prop->get_signature()->Name);
    }

    // Can just memcpy the array contents
    static_assert(std::is_trivially_copyable_v<FScriptDelegate>);

    auto arr = reinterpret_cast<TArray<FScriptDelegate>*>(addr);
    arr->resize(value.base->size());
    memcpy(arr->data, value.base->data, value.base->size() * sizeof(*arr->data));
}

void PropTraits<UMulticastDelegateProperty>::destroy(const UMulticastDelegateProperty* /*prop*/,
                                                     uintptr_t addr) {
    reinterpret_cast<TArray<FScriptDelegate>*>(addr)->free();
}

}  // namespace unrealsdk::unreal
