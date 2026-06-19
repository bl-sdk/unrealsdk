#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/wrappers/wrapped_inline_struct.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fgbxinlinestruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

WrappedInlineStruct::WrappedInlineStruct(const FGbxInlineStruct* inline_ref,
                                         const UnrealPointer<void>& parent)
    : WrappedStruct(inline_ref->ptr.obj->get_type(), inline_ref->ptr.obj, parent),
      inline_ref(*inline_ref) {
    // Add a ref for ourselves
    if (inline_ref->ptr.controller != nullptr) {
        inline_ref->ptr.controller->ref_count++;
    }
}

WrappedInlineStruct::WrappedInlineStruct(const WrappedInlineStruct& other)
    : WrappedStruct(other.type, nullptr) {
    auto* controller = unrealsdk::u_malloc<internal::FGbxInlineStructAllocation>(
        internal::FGbxInlineStructAllocation::EXTRA_ALLOC_SIZE + this->type->get_struct_size());

    controller->controller.vftable = other.inline_ref.ptr.controller->vftable;
    controller->controller.ref_count = 1;
    controller->controller.weak_ref_count = 1;
    controller->offset_to_data = internal::FGbxInlineStructAllocation::EXTRA_ALLOC_SIZE;
    controller->unknown = 0;
    controller->def.vftable = other.inline_ref.ptr.obj->vftable;
    static_assert(sizeof(controller->def) == sizeof(controller->def.vftable),
                  "unexpected extra data to initialize in a FGbxDef");

    this->inline_ref.ptr.controller = &controller->controller;
    this->inline_ref.ptr.obj = &controller->def;
    this->inline_ref.flags = other.inline_ref.flags;

    copy_struct(reinterpret_cast<uintptr_t>(this->inline_ref.ptr.obj), other);
}

WrappedInlineStruct::WrappedInlineStruct(WrappedInlineStruct&& other) noexcept
    : WrappedStruct(std::move(other)), inline_ref(std::exchange(other.inline_ref, {})) {}

WrappedInlineStruct& WrappedInlineStruct::operator=(const WrappedInlineStruct& other) {
    // I don't think we need to mess with inline_ref here. We're only copying the struct, so no need
    // to mess with the smart pointer at all. And the types are the same, so no need to update the
    // struct vftable. Not fully confident though.
    WrappedStruct::operator=(other);
    return *this;
}
WrappedInlineStruct& WrappedInlineStruct::operator=(WrappedInlineStruct&& other) noexcept {
    std::swap(this->inline_ref, other.inline_ref);
    WrappedStruct::operator=(std::move(other));
    return *this;
}

/**
 * @brief Destroys the wrapped struct
 */
WrappedInlineStruct::~WrappedInlineStruct() {
    // Remove a ref, which may free the struct for us
    if (this->inline_ref.ptr.controller != nullptr) {
        this->inline_ref.ptr.controller->remove_strong_ref();
    }
    // And zero the pointers, to prevent our other logic trying to free it too
    this->inline_ref.ptr = {.obj = nullptr, .controller = nullptr};
    this->base = nullptr;
}

}  // namespace unrealsdk::unreal
