#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/property_proxy.h"

namespace unrealsdk::unreal {

PropertyProxy::PropertyProxy(UProperty* prop) : prop(prop), value(nullptr) {}

[[nodiscard]] bool PropertyProxy::has_value(void) const {
    return this->value != nullptr;
}

}  // namespace unrealsdk::unreal
