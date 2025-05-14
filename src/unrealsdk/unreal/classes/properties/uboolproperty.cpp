#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UBoolProperty, UNREALSDK_UBOOLPROPERTY_FIELDS);

PropTraits<UBoolProperty>::Value PropTraits<UBoolProperty>::get(
    const UBoolProperty* prop,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto mask = prop->FieldMask();
    auto* bitfield = reinterpret_cast<decltype(mask)*>(addr);

    return (*bitfield & mask) != 0;
}

void PropTraits<UBoolProperty>::set(const UBoolProperty* prop, uintptr_t addr, const Value& value) {
    auto mask = prop->FieldMask();
    auto* bitfield = reinterpret_cast<decltype(mask)*>(addr);

    if (value) {
        *bitfield |= mask;
    } else {
        *bitfield &= ~mask;
    }
}

}  // namespace unrealsdk::unreal
