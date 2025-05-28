#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal::impl {

size_t UnrealPointerControl::inc_ref(void) {
    if (this->refs == std::numeric_limits<size_t>::max()) {
        throw std::runtime_error("Unreal smart pointer reached maximum references!");
    }
    return ++this->refs;
}

size_t UnrealPointerControl::dec_ref(void) {
    if (this->refs == 0) {
        throw std::runtime_error(
            "Tried to decrement reference from unreal smart pointer already at zero!");
    }
    return --this->refs;
}

void UnrealPointerControl::destroy_object(void) {
    switch (this->pointer_type) {
        case PointerType::STRUCT: {
            auto struct_type = this->metadata.struct_type;
            if (struct_type != nullptr) {
                // Need to reconstruct the base address, since our pointer may be elsewhere
                auto struct_base =
                    reinterpret_cast<uintptr_t>(this) + sizeof(impl::UnrealPointerControl);

                destroy_struct(struct_type, struct_base);
            }
            return;
        }

        case PointerType::PROPERTY: {
            auto prop = this->metadata.prop;
            if (prop != nullptr) {
                // Need to reconstruct the base address, since our pointer may be elsewhere
                auto prop_base = reinterpret_cast<uintptr_t>(this)
                                 + sizeof(impl::UnrealPointerControl) - prop->Offset_Internal();

                cast(prop, [prop_base]<typename T>(const T* prop) {
                    for (size_t i = 0; i < (size_t)prop->ArrayDim(); i++) {
                        destroy_property<T>(prop, i, prop_base);
                    }
                });
            }
            return;
        }
    }
}

}  // namespace unrealsdk::unreal::impl
