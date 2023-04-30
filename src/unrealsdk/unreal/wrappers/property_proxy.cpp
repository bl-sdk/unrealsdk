#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/property_proxy.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

/*
Property proxies are primarily intended to hold hook return values.
We generally expect these to be small values, such as bools, enums/ints, or object pointers, which
would be a bit off a waste to allocate space on the heap for.
Instead, we take inspiration from short string optimization, and store short properties inside the
value pointer.
We need to access the property to get it's offset whenever we try read/write the value anyway, so
also checking size doesn't even add any extra pointer accesses.
*/

PropertyProxy::PropertyProxy(UProperty* prop) : prop(prop), value(nullptr), been_set(false) {
    if (prop != nullptr) {
        const size_t size = this->prop->ElementSize * this->prop->ArrayDim;
        if (size > sizeof(void*)) {
            this->value = unrealsdk::u_malloc(size);
        }
    }
}
PropertyProxy::PropertyProxy(const PropertyProxy& other) : PropertyProxy(other.prop) {
    if (other.been_set) {
        cast_prop(this->prop, [this, other]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                this->set<T>(i, other.get<T>(i));
            }
        });
    }
}
PropertyProxy::PropertyProxy(PropertyProxy&& other) noexcept
    : prop(std::exchange(other.prop, nullptr)),
      value(std::exchange(other.value, nullptr)),
      been_set(std::exchange(other.been_set, false)) {}

PropertyProxy& PropertyProxy::operator=(const PropertyProxy& other) {
    if (other.prop != this->prop) {
        throw std::runtime_error("Property proxy is not instance of "
                                 + (std::string)this->prop->Name);
    }
    if (this->prop != nullptr) {
        cast_prop(this->prop, [this, other]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                this->set<T>(i, other.get<T>(i));
            }
        });
    }
    return *this;
}
PropertyProxy& PropertyProxy::operator=(PropertyProxy&& other) noexcept {
    std::swap(this->prop, other.prop);
    std::swap(this->value, other.value);
    std::swap(this->been_set, other.been_set);
    return *this;
}

PropertyProxy::~PropertyProxy() {
    if (this->prop == nullptr) {
        // Nothing to do
        return;
    }

    auto addr = this->get_value_addr();
    cast_prop(this->prop, [addr]<typename T>(const T* prop) {
        for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
            destroy_property<T>(prop, i, addr);
        }
    });

    const size_t size = this->prop->ElementSize * this->prop->ArrayDim;
    if (size > sizeof(void*)) {
        unrealsdk::u_free(this->value);
    }
}

uintptr_t PropertyProxy::get_value_addr(void) const {
    if (this->prop == nullptr) {
        throw std::runtime_error("Property does not exist!");
    }
    const size_t size = this->prop->ElementSize * this->prop->ArrayDim;

    return (size > sizeof(void*) ? reinterpret_cast<uintptr_t>(this->value)
                                 : reinterpret_cast<uintptr_t>(&this->value))
           - this->prop->Offset_Internal;
}

bool PropertyProxy::has_value(void) const {
    return this->been_set;
}

void PropertyProxy::destroy(void) {
    // We're lying a little here, we'll simply act as if we don't have a value set, and only
    // actually destroy it in the destructor
    // In case of fixed arrays, just going to consider uninitialized entries to have an undefined
    // value, so doing `destroy(); set(0, ...); get(1);` is your own fault.
    this->been_set = false;
}

void PropertyProxy::copy_to(uintptr_t addr) {
    if (!this->been_set) {
        throw std::runtime_error("Cannot copy empty property!");
    }
    cast_prop(this->prop, [this, addr]<typename T>(const T* prop) {
        for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
            set_property<T>(prop, i, addr, this->get<T>(i));
        }
    });
}

void PropertyProxy::copy_from(uintptr_t addr) {
    if (this->prop == nullptr) {
        throw std::runtime_error("Property does not exist!");
    }

    cast_prop(this->prop, [this, addr]<typename T>(const T* prop) {
        for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
            this->set<T>(i, get_property(prop, i, addr));
        }
    });
}

}  // namespace unrealsdk::unreal
