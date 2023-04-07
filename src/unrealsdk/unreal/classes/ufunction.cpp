#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"

namespace unrealsdk::unreal {

UProperty* UFunction::find_return_param(void) const {
    for (auto prop : this->properties()) {
        if ((prop->PropertyFlags & UProperty::PROP_FLAG_RETURN) != 0) {
            return prop;
        }
    }
    return nullptr;
}

}  // namespace unrealsdk::unreal
