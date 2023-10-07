#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uweakobjectproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/structs/fweakobjectptr.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

PropTraits<UWeakObjectProperty>::Value PropTraits<UWeakObjectProperty>::get(
    const UWeakObjectProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto ptr = reinterpret_cast<FWeakObjectPtr*>(addr);
    return unrealsdk::gobjects().get_weak_object(ptr);
}

void PropTraits<UWeakObjectProperty>::set(const UWeakObjectProperty* prop,
                                          uintptr_t addr,
                                          const Value& value) {
    // Ensure the object is of a valid class
    if (value != nullptr) {
        auto prop_cls = prop->get_property_class();
        if (!value->is_instance(prop_cls)) {
            throw std::runtime_error("Object is not instance of " + (std::string)prop_cls->Name);
        }
    }

    auto ptr = reinterpret_cast<FWeakObjectPtr*>(addr);
    unrealsdk::gobjects().set_weak_object(ptr, value);
}

}  // namespace unrealsdk::unreal
