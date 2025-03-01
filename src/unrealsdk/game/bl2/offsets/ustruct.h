#ifndef UNREALSDK_GAME_BL2_OFFSETS_USTRUCT_H
#define UNREALSDK_GAME_BL2_OFFSETS_USTRUCT_H

#include "unrealsdk/game/bl2/offsets/ufield.h"
#include "unrealsdk/unreal/structs/tarray.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UProperty;
class UStruct;
class UField;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl2 {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class UStruct : public bl2::UField {
   private:
    uint8_t UnknownData00[0x8];

   public:
    unreal::UStruct* SuperField;
    unreal::UField* Children;

   private:
    uint16_t PropertySize;
    uint8_t UnknownData01[0x1A];

   public:
    unreal::UProperty* PropertyLink;

   private:
    uint8_t UnknownData02[0x10];

    unreal::TArray<unreal::UObject*> ScriptObjectReferences;
};

// NOLINTEND(readability-identifier-naming, readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::game::bl2

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_USTRUCT_H */
