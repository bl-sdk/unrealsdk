#ifndef UNREALSDK_GAME_BL3_OFFSETS_UPROPERTY_H
#define UNREALSDK_GAME_BL3_OFFSETS_UPROPERTY_H

#include "unrealsdk/game/bl3/offsets/ufield.h"
#include "unrealsdk/unreal/structs/fname.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UProperty;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl3 {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class UProperty : public bl3::UField {
   public:
    int32_t ArrayDim;
    int32_t ElementSize;
    uint64_t PropertyFlags;

   private:
    uint16_t RepIndex;
    uint8_t BlueprintReplicationCondition;

   public:
    int32_t Offset_Internal;

   private:
    unreal::FName RepNotifyFunc;

   public:
    unreal::UProperty* PropertyLinkNext;

   private:
    unreal::UProperty* NextRef;
    unreal::UProperty* DestructorLinkNext;
    unreal::UProperty* PostConstructLinkNext;
};

// NOLINTEND(readability-identifier-naming, readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace unrealsdk::game::bl3

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_USTRUCT_H */
