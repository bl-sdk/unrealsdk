#include "pch.h"

#include "unreal/classes/properties/uobjectproperty.h"
#include "unreal/classes/uclass.h"

namespace unrealsdk::unreal {

PropTraits<UObjectProperty>::Value PropTraits<UObjectProperty>::get(const UObjectProperty* /*prop*/,
                                                                    uintptr_t addr) {
    return *reinterpret_cast<Value*>(addr);
}

void PropTraits<UObjectProperty>::set(const UObjectProperty* prop, uintptr_t addr, Value value) {
    // Ensure the object is of a valid class
    if (value != nullptr) {
        auto prop_cls = prop->read_field(&UObjectProperty::PropertyClass);
        if (!value->is_instance(prop_cls)) {
            throw std::runtime_error("Object is not instance of " + (std::string)prop_cls->Name);
        }
    }

    *reinterpret_cast<Value*>(addr) = value;
}

}  // namespace unrealsdk::unreal