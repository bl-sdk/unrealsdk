#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

namespace {

/**
 * @brief Allocates a block of memory to hold a struct, with an appropriate deleter.
 *
 * @param type The type to allocate a struct of.
 * @return A shared pointer to the block of memory, or nullptr if the struct is empty.
 */
[[nodiscard]] UnrealPointer<void> alloc_struct(const UStruct* type) {
    auto size = type->get_struct_size();
    if (size == 0) {
        return {nullptr};
    }
    return {size};
}

}  // namespace

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

WrappedStruct::WrappedStruct(const UStruct* type) : type(type), base(alloc_struct(type)) {}

WrappedStruct::WrappedStruct(const UStruct* type, void* base, const UnrealPointer<void>& parent)
    : type(type), base(parent, base) {}

WrappedStruct::WrappedStruct(const WrappedStruct& other)
    : type(other.type), base(alloc_struct(other.type)) {
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

}  // namespace unrealsdk::unreal
