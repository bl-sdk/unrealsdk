#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

PropTraits<UClassProperty>::Value PropTraits<UClassProperty>::get(
    const UClassProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    return *reinterpret_cast<Value*>(addr);
}

void PropTraits<UClassProperty>::set(const UClassProperty* prop,
                                     uintptr_t addr,
                                     const Value& value) {
    // Ensure the object is of a valid class
    if (value != nullptr) {
        auto prop_cls = prop->get_property_class();
        if (!value->is_instance(prop_cls)) {
            throw std::runtime_error("Object is not instance of " + (std::string)prop_cls->Name);
        }
        auto meta_cls = prop->read_field(&UClassProperty::MetaClass);
        if (!value->inherits(meta_cls)) {
            throw std::runtime_error("Class does not inherit from " + (std::string)meta_cls->Name);
        }
    }

    *reinterpret_cast<Value*>(addr) = value;
}

}  // namespace unrealsdk::unreal
