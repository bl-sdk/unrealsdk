#ifndef UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_FUNCS_H
#define UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_FUNCS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

template <typename T>
void UnrealPointer<T>::release(void) {
    auto old_control = this->control;
    this->control = nullptr;
    this->ptr = nullptr;

    // If we had a control block, we owned this pointer, and need to delete it
    // If dec_ref throws, we can't be sure there aren't other references to the control block,
    // so we can't really do anything, better to potentially leak than free something used
    // elsewhere
    if (old_control != nullptr && old_control->dec_ref() == 0) {
        // If the destructors throw, we still want to free the memory
        try {
            // Destroy the object first, since it might allocate more memory, we know it's less
            // catastrophic to miss the control block destructor
            old_control->destroy_object();

            // Since we're using placement new, we need to manually call the destructor
            old_control->~UnrealPointerControl();
        } catch (const std::exception& ex) {
            unrealsdk::u_free(old_control);
            LOG(ERROR, "Exception in unreal pointer destructor: {}", ex.what());
            throw;
        } catch (...) {
            unrealsdk::u_free(old_control);
            LOG(ERROR, "Unknown exception in unreal pointer destructor");
            throw;
        }
        unrealsdk::u_free(old_control);
    }
}

template <typename T>
UnrealPointer<T>::UnrealPointer(const UStruct* struct_type)
    requires std::is_void_v<T>
    : control(nullptr), ptr(nullptr) {
    // If malloc throws, it should have handled freeing memory if required
    auto buf =
        unrealsdk::u_malloc(struct_type->get_struct_size() + sizeof(impl::UnrealPointerControl));

    // Otherwise, if we throw during initialization we need to free manually
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        this->control = new (buf) impl::UnrealPointerControl(struct_type);

        this->ptr = reinterpret_cast<void*>(this->control + 1);
    } catch (const std::exception& ex) {
        unrealsdk::u_free(buf);
        LOG(ERROR, "Exception in unreal pointer constructor: {}", ex.what());
        throw;
    } catch (...) {
        unrealsdk::u_free(buf);
        LOG(ERROR, "Unknown exception in unreal pointer constructor");
        throw;
    }

    // Add our ref
    this->attach();
}

template <typename T>
UnrealPointer<T>::UnrealPointer(const UProperty* prop)
    requires std::is_void_v<T>
    : control(nullptr), ptr(nullptr) {
    // If malloc throws, it should have handled freeing memory if required
    auto buf = unrealsdk::u_malloc((prop->ElementSize * prop->ArrayDim)
                                   + sizeof(impl::UnrealPointerControl));

    // Otherwise, if we throw during initialization we need to free manually
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        this->control = new (buf) impl::UnrealPointerControl(prop);

        this->ptr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this->control + 1)
                                            - prop->Offset_Internal);
    } catch (const std::exception& ex) {
        unrealsdk::u_free(buf);
        LOG(ERROR, "Exception in unreal pointer constructor: {}", ex.what());
        throw;
    } catch (...) {
        unrealsdk::u_free(buf);
        LOG(ERROR, "Unknown exception in unreal pointer constructor");
        throw;
    }

    // Add our ref
    this->attach();
}

}  // namespace unrealsdk::unreal
#endif /* UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_FUNCS_H */
