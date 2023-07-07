#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

decltype(UBoolProperty::FieldMask) UBoolProperty::get_field_mask(void) const {
    return this->read_field(&UBoolProperty::FieldMask);
}

PropTraits<UBoolProperty>::Value PropTraits<UBoolProperty>::get(
    const UBoolProperty* prop,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto mask = prop->get_field_mask();
    auto* bitfield = reinterpret_cast<decltype(mask)*>(addr);

    return (*bitfield & mask) != 0;
}

void PropTraits<UBoolProperty>::set(const UBoolProperty* prop, uintptr_t addr, const Value& value) {
    auto mask = prop->get_field_mask();
    auto* bitfield = reinterpret_cast<decltype(mask)*>(addr);

    if (value) {
        *bitfield |= mask;
    } else {
        *bitfield &= ~mask;
    }
}

}  // namespace unrealsdk::unreal
