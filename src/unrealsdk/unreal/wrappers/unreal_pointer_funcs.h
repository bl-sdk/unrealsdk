#ifndef UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_FUNCS_H
#define UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_FUNCS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

template <typename T>
void UnrealPointer<T>::release(void) {
    auto old_control = this->control;
    auto old_ptr = this->ptr;
    this->control = nullptr;
    this->ptr = nullptr;

    // If dec_ref throws, we can't be sure there aren't other references to the control block,
    // so we can't really do anything, better to potentially leak than free something used
    // elsewhere
    if (old_control != nullptr && old_control->dec_ref() == 0) {
        // If the destructors throw, we still want to free the memory
        try {
            // Destroy the struct first, since we know it's less catastrophic to miss the control
            // block destructor
            destroy_struct(old_control->deleter_struct, reinterpret_cast<uintptr_t>(old_ptr));

            // Since we're using placement new, we need to manually call the destructor
            old_control->~UnrealPointerControl();
        } catch (...) {
            unrealsdk::u_free(old_control);
            throw;
        }
        unrealsdk::u_free(old_control);
    }
}

template <typename T>
UnrealPointer<T>::UnrealPointer(const UStruct* struct_type) : control(nullptr), ptr(nullptr) {
    // If malloc throws, it should have handled freeing memory if required
    auto buf =
        unrealsdk::u_malloc(struct_type->get_struct_size() + sizeof(impl::UnrealPointerControl));

    // Otherwise, if we throw during initialization we need to free manually
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        this->control = new (buf) impl::UnrealPointerControl(struct_type);

        this->ptr = reinterpret_cast<T*>(this->control + 1);
    } catch (...) {
        unrealsdk::u_free(buf);
        throw;
    }

    // Add our ref
    this->attach();
}

}  // namespace unrealsdk::unreal
#endif /* UNREALSDK_SRC_UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_FUNCS_H */
