#ifndef UNREALSDK_UNREAL_STRUCTS_FWEAKOBJECTPTR_H
#define UNREALSDK_UNREAL_STRUCTS_FWEAKOBJECTPTR_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

// If you want hold a weak reference in your own code, use the `WeakPointer` wrapper class.

// If you have to interact with one of these pulled from unreal internals, use the getters/setters
// on the GObjects wrapper. Weak pointers are dependent on the internal GObjects layout, so better
// to implement it there.

struct FWeakObjectPtr {
    // Default constructs to a null reference
    int32_t object_index = -1;
    int32_t object_serial_number = 0;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FWEAKOBJECTPTR_H */
