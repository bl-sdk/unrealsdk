#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"
#include "unrealsdk/unreal/structs/tpersistentobjectptr.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UClass* USoftClassProperty::get_meta_class(void) const {
    return this->read_field(&USoftClassProperty::MetaClass);
}

PropTraits<ULazyObjectProperty>::Value PropTraits<ULazyObjectProperty>::get(
    const ULazyObjectProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto ptr = reinterpret_cast<FLazyObjectPtr*>(addr);
    return unrealsdk::gobjects().get_weak_object(&ptr->weak_ptr);
}
PropTraits<USoftObjectProperty>::Value PropTraits<USoftObjectProperty>::get(
    const USoftObjectProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto ptr = reinterpret_cast<FSoftObjectPtr*>(addr);
    return unrealsdk::gobjects().get_weak_object(&ptr->weak_ptr);
}
PropTraits<USoftClassProperty>::Value PropTraits<USoftClassProperty>::get(
    const USoftClassProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    // Soft classes still use a soft object pointer struct
    auto ptr = reinterpret_cast<FSoftObjectPtr*>(addr);
    return reinterpret_cast<UClass*>(unrealsdk::gobjects().get_weak_object(&ptr->weak_ptr));
}

void PropTraits<ULazyObjectProperty>::set(const ULazyObjectProperty* prop,
                                          uintptr_t addr,
                                          const Value& value) {
    // Ensure the object is of a valid class
    if (value != nullptr) {
        auto prop_cls = prop->get_property_class();
        if (!value->is_instance(prop_cls)) {
            throw std::runtime_error("Object is not instance of " + (std::string)prop_cls->Name);
        }
    }

    unrealsdk::internal::flazyobjectptr_assign(reinterpret_cast<FLazyObjectPtr*>(addr), value);
}
void PropTraits<USoftObjectProperty>::set(const USoftObjectProperty* prop,
                                          uintptr_t addr,
                                          const Value& value) {
    // Ensure the object is of a valid class
    if (value != nullptr) {
        auto prop_cls = prop->get_property_class();
        if (!value->is_instance(prop_cls)) {
            throw std::runtime_error("Object is not instance of " + (std::string)prop_cls->Name);
        }
    }

    unrealsdk::internal::fsoftobjectptr_assign(reinterpret_cast<FSoftObjectPtr*>(addr), value);
}
void PropTraits<USoftClassProperty>::set(const USoftClassProperty* prop,
                                         uintptr_t addr,
                                         const Value& value) {
    // Ensure the object is of a valid class
    if (value != nullptr) {
        auto prop_cls = prop->get_property_class();
        if (!value->is_instance(prop_cls)) {
            throw std::runtime_error("Object is not instance of " + (std::string)prop_cls->Name);
        }
        auto meta_cls = prop->get_meta_class();
        if (!value->inherits(meta_cls)) {
            throw std::runtime_error("Class does not inherit from " + (std::string)meta_cls->Name);
        }
    }

    unrealsdk::internal::fsoftobjectptr_assign(reinterpret_cast<FSoftObjectPtr*>(addr), value);
}

void PropTraits<USoftObjectProperty>::destroy(const USoftObjectProperty* /*prop*/, uintptr_t addr) {
    reinterpret_cast<FSoftObjectPtr*>(addr)->identifier.subpath.free();
}
void PropTraits<USoftClassProperty>::destroy(const USoftClassProperty* /*prop*/, uintptr_t addr) {
    reinterpret_cast<FSoftObjectPtr*>(addr)->identifier.subpath.free();
}

}  // namespace unrealsdk::unreal
