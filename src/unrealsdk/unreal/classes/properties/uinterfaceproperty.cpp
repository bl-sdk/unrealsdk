#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/fimplementedinterface.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UInterfaceProperty, UNREALSDK_UINTERFACEPROPERTY_FIELDS);

struct FScriptInterface {
    UObject* obj;     // A pointer to a UObject that implements a native interface.
    void* iface_ptr;  // Pointer to the location of the interface object within the UObject
                      // referenced by ObjectPointer.
};

PropTraits<UInterfaceProperty>::Value PropTraits<UInterfaceProperty>::get(
    const UInterfaceProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    return reinterpret_cast<FScriptInterface*>(addr)->obj;
}

void PropTraits<UInterfaceProperty>::set(const UInterfaceProperty* prop,
                                         uintptr_t addr,
                                         const Value& value) {
    auto prop_iface = prop->InterfaceClass();

    size_t pointer_offset = 0;

    // Ensure the object is of a valid class
    if (value != nullptr) {
        FImplementedInterface impl{};
        if (!value->is_implementation(prop_iface, &impl)) {
            throw std::runtime_error("Object is not implementation of "
                                     + (std::string)prop_iface->Name());
        }
        pointer_offset = impl.get_pointer_offset();
    }

    auto iface = reinterpret_cast<FScriptInterface*>(addr);
    iface->obj = value;
    // If value is null, offset will stay 0, so we also null the pointer
    iface->iface_ptr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(value) + pointer_offset);
}

}  // namespace unrealsdk::unreal
