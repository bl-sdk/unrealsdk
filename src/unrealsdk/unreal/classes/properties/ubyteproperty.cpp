#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"

namespace unrealsdk::unreal {

UEnum* UByteProperty::get_enum(void) const {
    return this->read_field(&UByteProperty::Enum);
}

}  // namespace unrealsdk::unreal
