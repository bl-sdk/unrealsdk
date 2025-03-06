#ifndef UNREALSDK_GAME_BL1_OFFSETS_H
#define UNREALSDK_GAME_BL1_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/offsets.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

struct FImplementedInterface;

}

namespace unrealsdk::game::bl1 {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UStruct;
class UClass;

using UObject = bl2::generic::UObject<UClass>;

class UField : public UObject {
   public:
    // Superfield is considered to be part of UStruct in the rest of the sdk
    // Luckily, we can just expose it here and it'll all get picked up right
    UStruct* SuperField;
    UField* Next;
};

class UProperty : public UField {
   public:
    int32_t ArrayDim;
    int32_t ElementSize;
    uint32_t PropertyFlags;

   private:
    uint8_t UnknownData00[0x14];

   public:
    int32_t Offset_Internal;
    UProperty* PropertyLinkNext;

   private:
    uint8_t UnknownData01[0x20];
};

class UStruct : public UField {
   private:
    uint8_t UnknownData00[0x08];

   public:
    UField* Children;
    uint16_t PropertySize;

   private:
    uint8_t UnknownData01[0x1C + 0x02];

   public:
    UProperty* PropertyLink;

   private:
    uint8_t UnknownData02[0x10];

   public:
    unreal::TArray<UObject*> ScriptObjectReferences;

   private:
    uint8_t UnknownData03[0x04];
};

class UClass : public UStruct {
   private:
    uint8_t UnknownData00[0xC0];

   public:
    UObject* ClassDefaultObject;

   private:
    uint8_t UnknownData01[0x48];

   public:
    unreal::TArray<unreal::FImplementedInterface> Interfaces;
};

using UScriptStruct = unreal::offsets::generic::UScriptStruct<UStruct>;
using UFunction = bl2::generic::UFunction<UStruct>;

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::game::bl1

#endif

#endif /* UNREALSDK_GAME_BL1_OFFSETS_H */
