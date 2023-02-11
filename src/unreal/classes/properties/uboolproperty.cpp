#include "pch.h"

#include "unreal/classes/properties/uboolproperty.h"

namespace unrealsdk::unreal {

PropTraits<UBoolProperty>::Value PropTraits<UBoolProperty>::get(UBoolProperty* prop,
                                                                uintptr_t addr) {
    auto mask = prop->read_field(&UBoolProperty::FieldMask);
    auto* bitfield = reinterpret_cast<decltype(UBoolProperty::FieldMask)*>(addr);

    return (*bitfield & mask) != 0;
}

void PropTraits<UBoolProperty>::set(UBoolProperty* prop, uintptr_t addr, Value value) {
    auto mask = prop->read_field(&UBoolProperty::FieldMask);
    auto* bitfield = reinterpret_cast<decltype(UBoolProperty::FieldMask)*>(addr);

    if (value) {
        *bitfield |= mask;
    } else {
        *bitfield &= ~mask;
    }
}

}  // namespace unrealsdk::unreal
