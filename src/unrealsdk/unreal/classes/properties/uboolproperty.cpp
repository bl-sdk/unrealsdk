#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

PropTraits<UBoolProperty>::Value PropTraits<UBoolProperty>::get(
    const UBoolProperty* prop,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto mask = prop->read_field(&UBoolProperty::FieldMask);
    auto* bitfield = reinterpret_cast<decltype(UBoolProperty::FieldMask)*>(addr);

    return (*bitfield & mask) != 0;
}

void PropTraits<UBoolProperty>::set(const UBoolProperty* prop, uintptr_t addr, const Value& value) {
    auto mask = prop->read_field(&UBoolProperty::FieldMask);
    auto* bitfield = reinterpret_cast<decltype(UBoolProperty::FieldMask)*>(addr);

    if (value) {
        *bitfield |= mask;
    } else {
        *bitfield &= ~mask;
    }
}

}  // namespace unrealsdk::unreal
