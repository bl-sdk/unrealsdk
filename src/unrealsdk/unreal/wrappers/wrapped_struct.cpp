#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

void copy_struct(uintptr_t dest, const WrappedStruct& src) {
    for (const auto& prop : src.type->properties()) {
        cast_prop(prop, [dest, &src]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                set_property<T>(prop, i, dest, src.get<T>(prop, i));
            }
        });
    }
}

void destroy_struct(const UStruct* type, uintptr_t addr) {
    for (const auto& prop : type->properties()) {
        cast_prop(prop, [addr]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                destroy_property<T>(prop, i, addr);
            }
        });
    }
}

WrappedStruct::WrappedStruct(const UStruct* type) : type(type), base(type) {}

WrappedStruct::WrappedStruct(const UStruct* type, void* base, const UnrealPointer<void>& parent)
    : type(type), base(parent, base) {}

WrappedStruct::WrappedStruct(const WrappedStruct& other) : type(other.type), base(other.type) {
    if (this->base != nullptr && other.base != nullptr) {
        copy_struct(reinterpret_cast<uintptr_t>(this->base.get()), other);
    }
}

WrappedStruct::WrappedStruct(WrappedStruct&& other) noexcept
    : type(std::exchange(other.type, nullptr)), base(std::exchange(other.base, {nullptr})) {}

WrappedStruct& WrappedStruct::operator=(const WrappedStruct& other) {
    if (other.type != this->type) {
        throw std::runtime_error("Struct is not an instance of " + (std::string)this->type->Name);
    }
    if (this->base != nullptr && other.base != nullptr) {
        copy_struct(reinterpret_cast<uintptr_t>(this->base.get()), other);
    }
    return *this;
}
WrappedStruct& WrappedStruct::operator=(WrappedStruct&& other) noexcept {
    std::swap(this->type, other.type);
    std::swap(this->base, other.base);
    return *this;
}

WrappedStruct WrappedStruct::copy_params_only(void) const {
    WrappedStruct new_struct{this->type};
    if (this->base == nullptr || new_struct.base == nullptr) {
        return new_struct;
    }

    auto dest = reinterpret_cast<uintptr_t>(new_struct.base.get());
    for (const auto& prop : this->type->properties()) {
        if ((prop->PropertyFlags & UProperty::PROP_FLAG_PARAM) == 0) {
            continue;
        }

        cast_prop(prop, [dest, this]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                set_property<T>(prop, i, dest, this->get<T>(prop, i));
            }
        });
    }

    return new_struct;
}

}  // namespace unrealsdk::unreal
