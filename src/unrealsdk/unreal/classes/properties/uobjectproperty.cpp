#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UObjectProperty, UNREALSDK_UOBJECTPROPERTY_FIELDS);

PropTraits<UObjectProperty>::Value PropTraits<UObjectProperty>::get(
    const UObjectProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    return *reinterpret_cast<Value*>(addr);
}

void PropTraits<UObjectProperty>::set(const UObjectProperty* prop,
                                      uintptr_t addr,
                                      const Value& value) {
    // Ensure the object is of a valid class
    if (value != nullptr) {
        auto prop_cls = prop->PropertyClass();
        if (!value->is_instance(prop_cls)) {
            throw std::runtime_error("Object is not instance of " + (std::string)prop_cls->Name());
        }
    }

    *reinterpret_cast<Value*>(addr) = value;
}

}  // namespace unrealsdk::unreal
