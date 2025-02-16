#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/property_proxy.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

PropertyProxy::PropertyProxy(UProperty* prop) : prop(prop), ptr(nullptr) {}
PropertyProxy::PropertyProxy(const PropertyProxy& other) : prop(other.prop), ptr(nullptr) {
    if (this->prop != nullptr && other.has_value()) {
        cast(this->prop, [this, &other]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                this->set<T>(i, other.get<T>(i));
            }
        });
    }
}
PropertyProxy& PropertyProxy::operator=(const PropertyProxy& other) {
    if (other.prop != this->prop) {
        throw std::runtime_error("Property proxy is not instance of "
                                 + (std::string)this->prop->Name());
    }
    if (this->prop != nullptr) {
        cast(this->prop, [this, &other]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                this->set<T>(i, other.get<T>(i));
            }
        });
    }
    return *this;
}

bool PropertyProxy::has_value(void) const {
    return this->ptr.get() != nullptr;
}

void PropertyProxy::destroy(void) {
    this->ptr = UnrealPointer<void>{nullptr};
}

void PropertyProxy::copy_to(uintptr_t addr) const {
    if (!this->has_value()) {
        throw std::runtime_error("Cannot copy empty property!");
    }
    if (this->prop != nullptr) {
        cast(this->prop, [this, addr]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                set_property<T>(prop, i, addr, this->get<T>(i));
            }
        });
    }
}

void PropertyProxy::copy_from(uintptr_t addr) {
    if (this->prop == nullptr) {
        throw std::runtime_error("Property does not exist!");
    }

    cast(this->prop, [this, addr]<typename T>(const T* prop) {
        for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
            this->set<T>(i, get_property(prop, i, addr));
        }
    });
}

}  // namespace unrealsdk::unreal
