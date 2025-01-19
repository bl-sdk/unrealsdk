#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

decltype(UScriptStruct::StructFlags_internal)& UScriptStruct::StructFlags(void) {
    return this->get_field(&UScriptStruct::StructFlags_internal);
}
[[nodiscard]] const decltype(UScriptStruct::StructFlags_internal)& UScriptStruct::StructFlags(
    void) const {
    return this->get_field(&UScriptStruct::StructFlags_internal);
}

}  // namespace unrealsdk::unreal
