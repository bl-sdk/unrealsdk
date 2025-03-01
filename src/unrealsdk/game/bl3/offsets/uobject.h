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

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(readability-identifier-naming)

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct UObject {
   private:
    uintptr_t* vftable;

   public:
    int32_t ObjectFlags;
    int32_t InternalIndex;
    unreal::UClass* Class;
    unreal::FName Name;
    unreal::UObject* Outer;
};

// NOLINTEND(readability-identifier-naming)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace unrealsdk::game::bl3

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_UOBJECT_H */
