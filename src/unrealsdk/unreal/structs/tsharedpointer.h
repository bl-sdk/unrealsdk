#ifndef UNREALSDK_UNREAL_STRUCTS_TSHAREDPOINTER_H
#define UNREALSDK_UNREAL_STRUCTS_TSHAREDPOINTER_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

struct TReferenceController {
    uintptr_t* vftable;
    std::atomic<int32_t> ref_count;
    std::atomic<int32_t> weak_ref_count;

    static_assert(std::atomic<int32_t>::is_always_lock_free
                      && sizeof(std::atomic<int32_t>) == sizeof(int32_t)
                      && alignof(std::atomic<int32_t>) == alignof(int32_t),
                  "atomic int32_t reference counts might not be implemented in hardware");

    /**
     * @brief Removes a strong reference, potentially destroying the object + controller.
     *
     * @return True if the controller was destroyed.
     */
    bool remove_strong_ref(void);

    /**
     * @brief Removes a weak reference, potentially destroying the controller.
     *
     * @return True if the controller was destroyed.
     */
    bool remove_weak_ref(void);
};

template <typename T>
struct TSharedPointer {
    T* obj;
    TReferenceController* controller;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_TSHAREDPOINTER_H */
