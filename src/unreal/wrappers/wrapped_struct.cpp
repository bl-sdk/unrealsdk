#include "pch.h"

#include "unreal/cast_prop.h"
#include "unreal/classes/ustruct.h"
#include "unreal/prop_traits.h"
#include "unreal/wrappers/wrapped_struct.h"
#include "unrealsdk.h"

namespace unrealsdk::unreal {


void copy_struct(uintptr_t dest, const WrappedStruct& src) {
    for (const auto& prop : src.type->properties()) {
        cast_prop(prop, [dest, &src]<typename T>(const T* prop) {
            for (size_t i = 0; i < prop->ArrayDim; i++) {
                set_property<T>(prop, i, dest, src.get<T>(prop, i));
            }
        });
    }
}

void destroy_struct(const UStruct* type, uintptr_t addr) {
    for (const auto& prop : type->properties()) {
        cast_prop(prop, [addr]<typename T>(const T* prop) {
            for (size_t i = 0; i < prop->ArrayDim; i++) {
                destroy_property<T>(prop, i, addr);
            }
        });
    }
}

/**
 * @brief Allocates a block of memory to hold a struct, with an appropriate deleter.
 *
 * @param type The type to allocate a struct of.
 * @return A shared pointer to the block of memory.
 */
[[nodiscard]] static std::shared_ptr<void> alloc_struct(const UStruct* type) {
    auto deleter = [type](void* data) {
        destroy_struct(type, reinterpret_cast<uintptr_t>(data));
        unrealsdk::u_free(data);
    };

    return {unrealsdk::u_malloc(type->get_struct_size()), deleter};
}

WrappedStruct::WrappedStruct(const UStruct* type) : type(type), base(alloc_struct(type)) {}

WrappedStruct::WrappedStruct(const UStruct* type, void* base, const std::shared_ptr<void>& parent)
    : type(type), base(parent, base) {}

WrappedStruct::WrappedStruct(const WrappedStruct& other)
    : type(other.type), base(alloc_struct(other.type)) {
    copy_struct(reinterpret_cast<uintptr_t>(this->base.get()), other);
}

WrappedStruct::WrappedStruct(WrappedStruct&& other) noexcept
    : type(std::exchange(other.type, nullptr)), base(std::exchange(other.base, {nullptr})) {}

WrappedStruct& WrappedStruct::operator=(const WrappedStruct& other) {
    if (other.type != this->type) {
        throw std::runtime_error("Struct is not an instance of " + (std::string)this->type->Name);
    }
    copy_struct(reinterpret_cast<uintptr_t>(this->base.get()), other);
    return *this;
}
WrappedStruct& WrappedStruct::operator=(WrappedStruct&& other) noexcept {
    std::swap(this->type, other.type);
    std::swap(this->base, other.base);
    return *this;
}

}  // namespace unrealsdk::unreal
