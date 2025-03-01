#ifndef UNREALSDK_GAME_BL3_OFFSETS_USTRUCT_H
#define UNREALSDK_GAME_BL3_OFFSETS_USTRUCT_H

#include "unrealsdk/game/bl3/offsets/ufield.h"
#include "unrealsdk/unreal/structs/tarray.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UProperty;
class UStruct;
class UField;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl3 {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class UStruct : public bl3::UField {
   public:
    unreal::UStruct* SuperField;
    unreal::UField* Children;

   private:
    int32_t PropertySize;
    int32_t MinAlignment;
    unreal::TArray<uint8_t> Script;

   public:
    unreal::UProperty* PropertyLink;

   private:
    unreal::UProperty* RefLink;
    unreal::UProperty* DestructorLink;
    unreal::UProperty* PostConstructLink;
    unreal::TArray<unreal::UObject*> ScriptObjectReferences;
};

// NOLINTEND(readability-identifier-naming, readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace unrealsdk::game::bl3

#endif

#endif /* UNREALSDK_GAME_BL3_OFFSETS_USTRUCT_H */
