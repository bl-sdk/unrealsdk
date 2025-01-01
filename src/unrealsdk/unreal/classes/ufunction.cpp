#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"

namespace unrealsdk::unreal {

decltype(UFunction::FunctionFlags_internal)& UFunction::FunctionFlags(void) {
    return this->get_field(&UFunction::FunctionFlags_internal);
}
[[nodiscard]] const decltype(UFunction::FunctionFlags_internal)& UFunction::FunctionFlags(
    void) const {
    return this->get_field(&UFunction::FunctionFlags_internal);
}
decltype(UFunction::NumParams_internal)& UFunction::NumParams(void) {
    return this->get_field(&UFunction::NumParams_internal);
}
[[nodiscard]] const decltype(UFunction::NumParams_internal)& UFunction::NumParams(void) const {
    return this->get_field(&UFunction::NumParams_internal);
}
decltype(UFunction::ParamsSize_internal)& UFunction::ParamsSize(void) {
    return this->get_field(&UFunction::ParamsSize_internal);
}
[[nodiscard]] const decltype(UFunction::ParamsSize_internal)& UFunction::ParamsSize(void) const {
    return this->get_field(&UFunction::ParamsSize_internal);
}
decltype(UFunction::ReturnValueOffset_internal)& UFunction::ReturnValueOffset(void) {
    return this->get_field(&UFunction::ReturnValueOffset_internal);
}
[[nodiscard]] const decltype(UFunction::ReturnValueOffset_internal)& UFunction::ReturnValueOffset(
    void) const {
    return this->get_field(&UFunction::ReturnValueOffset_internal);
}

UProperty* UFunction::find_return_param(void) const {
    for (auto prop : this->properties()) {
        if ((prop->PropertyFlags & UProperty::PROP_FLAG_RETURN) != 0) {
            return prop;
        }
    }
    return nullptr;
}

}  // namespace unrealsdk::unreal
