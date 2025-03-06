#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UFunction, UNREALSDK_UFUNCTION_FIELDS);

UProperty* UFunction::find_return_param(void) const {
    for (auto prop : this->properties()) {
        if ((prop->PropertyFlags() & UProperty::PROP_FLAG_RETURN) != 0) {
            return prop;
        }
    }
    return nullptr;
}

}  // namespace unrealsdk::unreal
