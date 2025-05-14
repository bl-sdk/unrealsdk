#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/umulticastdelegateproperty.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_multicast_delegate.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UMulticastDelegateProperty,
                                    UNREALSDK_UMULTICASTDELEGATEPROPERTY_FIELDS);

PropTraits<UMulticastDelegateProperty>::Value PropTraits<UMulticastDelegateProperty>::get(
    const UMulticastDelegateProperty* prop,
    uintptr_t addr,
    const UnrealPointer<void>& parent) {
    return {prop->Signature(), reinterpret_cast<TArray<FScriptDelegate>*>(addr), parent};
}

void PropTraits<UMulticastDelegateProperty>::set(const UMulticastDelegateProperty* prop,
                                                 uintptr_t addr,
                                                 const Value& value) {
    if (value.signature != prop->Signature()) {
        throw std::runtime_error("Multicast delegate signature doesn't match existing signature of "
                                 + (std::string)prop->Signature()->Name());
    }

    auto arr = reinterpret_cast<TArray<FScriptDelegate>*>(addr);
    if (arr->data != nullptr && arr->data == value.base.get()->data) {
        LOG(DEV_WARNING,
            L"Refusing to set multicast delegate property {} to itself, at address {:p}",
            prop->get_path_name(), reinterpret_cast<void*>(addr));
        return;
    }

    // Can just memcpy the array contents
    static_assert(std::is_trivially_copyable_v<FScriptDelegate>);

    arr->resize(value.base->size());
    memcpy(arr->data, value.base->data, value.base->size() * sizeof(*arr->data));
}

void PropTraits<UMulticastDelegateProperty>::destroy(const UMulticastDelegateProperty* /*prop*/,
                                                     uintptr_t addr) {
    reinterpret_cast<TArray<FScriptDelegate>*>(addr)->free();
}

}  // namespace unrealsdk::unreal
