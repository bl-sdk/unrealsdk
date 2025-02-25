#ifndef UNREALSDK_GAME_BL3_OFFSETS_UOBJECT_H
#define UNREALSDK_GAME_BL3_OFFSETS_UOBJECT_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fname.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UObject;
class UClass;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl3 {

// NOLINTBEGIN(readability-identifier-naming)

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct UObject {
    int32_t ObjectFlags;
    int32_t InternalIndex;
    unreal::UClass* Class;
    unreal::FName Name;
    unreal::UObject* Outer;
};

// NOLINTEND(readability-identifier-naming)

}  // namespace unrealsdk::game::bl3

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_UOBJECT_H */
