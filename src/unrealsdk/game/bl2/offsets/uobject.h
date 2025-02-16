#ifndef UNREALSDK_GAME_BL2_OFFSETS_UOBJECT_H
#define UNREALSDK_GAME_BL2_OFFSETS_UOBJECT_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fname.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UObject;
class UClass;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl2 {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(readability-identifier-naming)

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct UObject {
    uintptr_t* vftable;
    void* HashNext;

    uint64_t ObjectFlags;

    void* HashOuterNext;
    void* StateFrame;
    unreal::UObject* _Linker;
    void* _LinkerIndex;

    int32_t InternalIndex;

    int32_t NetIndex;

    UObject* Outer;
    unreal::FName Name;
    unreal::UClass* Class;

    unreal::UObject* ObjectArchetype;
};

// NOLINTEND(readability-identifier-naming)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::game::bl2

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_UOBJECT_H */
