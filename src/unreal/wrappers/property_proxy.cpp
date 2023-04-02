#include "pch.h"

#include "unreal/cast_prop.h"
#include "unreal/classes/uproperty.h"
#include "unreal/prop_traits.h"
#include "unreal/wrappers/property_proxy.h"

namespace unrealsdk::unreal {

PropertyProxy::PropertyProxy(UProperty* prop) : prop(prop), value(nullptr) {}

uintptr_t PropertyProxy::get_value_addr(void) const {
    return this->get_value_addr(this->value.get());
}
uintptr_t PropertyProxy::get_value_addr(void* data) const {
    return reinterpret_cast<uintptr_t>(data) - this->prop->Offset_Internal;
}

bool PropertyProxy::has_value(void) const {
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
        for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
            set_property<T>(prop, i, addr, this->get<T>(i));
        }
    });
}

void PropertyProxy::copy_from(uintptr_t addr) {
    cast_prop(this->prop, [this, addr]<typename T>(const T* prop) {
        for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
            this->set<T>(i, get_property(prop, i, addr));
        }
    });
}

}  // namespace unrealsdk::unreal
