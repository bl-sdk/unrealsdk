#include "pch.h"

#include "unreal/cast_prop.h"
#include "unreal/classes/uproperty.h"
#include "unreal/prop_traits.h"
#include "unreal/wrappers/property_proxy.h"

namespace unrealsdk::unreal {

PropertyProxy::PropertyProxy(UProperty* prop) : prop(prop), value(nullptr) {}

[[nodiscard]] bool PropertyProxy::has_value(void) const {
    return this->value != nullptr;
}

void PropertyProxy::destroy(void) {
    this->value = {nullptr};
}

void PropertyProxy::copy_to(uintptr_t addr) {
    if (!this->has_value()) {
        throw std::runtime_error("Cannot copy empty property!");
    }
    cast_prop(this->prop, [this, addr]<typename T>(const T* prop) {
        for (size_t i = 0; i < prop->ArrayDim; i++) {
            set_property(prop, i, addr, this->get<T>(i));
        }
    });
}

}  // namespace unrealsdk::unreal
