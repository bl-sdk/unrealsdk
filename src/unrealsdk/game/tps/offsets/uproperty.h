#ifndef UNREALSDK_GAME_TPS_OFFSETS_UPROPERTY_H
#define UNREALSDK_GAME_TPS_OFFSETS_UPROPERTY_H

#include "unrealsdk/game/bl2/offsets/ufield.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UProperty;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::tps {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class UProperty : public bl2::UField {
   public:
    int32_t ArrayDim;
    int32_t ElementSize;
    uint32_t PropertyFlags;

   private:
    uint8_t UnknownData00[0x14];

   public:
    int32_t Offset_Internal;
    unreal::UProperty* PropertyLinkNext;

   private:
    uint8_t UnknownData01[0xC];
};

// NOLINTEND(readability-identifier-naming, readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::game::tps

#endif

#endif /* UNREALSDK_GAME_TPS_OFFSETS_UPROPERTY_H */
