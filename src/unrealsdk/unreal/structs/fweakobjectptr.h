#ifndef UNREALSDK_UNREAL_STRUCTS_FWEAKOBJECTPTR_H
#define UNREALSDK_UNREAL_STRUCTS_FWEAKOBJECTPTR_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

class UObject;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

// Use the getters/setters on the GObjects wrapper to interact with these
// Weak pointers are dependent on the internal GObjects layout, so better to implement there

struct FWeakObjectPtr {
    int32_t object_index;
    int32_t object_serial_number;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FWEAKOBJECTPTR_H */
