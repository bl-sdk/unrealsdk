#ifndef UNREALSDK_GAME_TPS_OFFSETS_H
#define UNREALSDK_GAME_TPS_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/unreal/offsets.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

namespace unrealsdk::game::tps {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UClass;

using UObject = bl2::generic::UObject<UClass>;
using UField = bl2::UField;

class UProperty : public UField {
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

class UStruct : public UField {
   private:
    uint8_t UnknownData00[0x8];

   public:
    UStruct* SuperField;
    UField* Children;

   private:
    uint16_t PropertySize;
    uint8_t UnknownData01[0x1A];

   public:
    UProperty* PropertyLink;

   private:
    uint8_t UnknownData02[0x4];

    unreal::TArray<UObject*> ScriptObjectReferences;
};

class UClass : public UStruct {
   private:
    uint8_t UnknownData00[0xCC];

   public:
    UObject* ClassDefaultObject;

   private:
    uint8_t UnknownData01[0x14];

   public:
    unreal::TArray<unreal::FImplementedInterface> Interfaces;
};

using UScriptStruct = unreal::offsets::generic::UScriptStruct<UStruct>;
using UFunction = bl2::generic::UFunction<UStruct>;
using UConst = bl2::UConst;
using UEnum = bl2::UEnum;

using UArrayProperty = unreal::offsets::generic::UArrayProperty<UProperty>;
using UBoolProperty = bl2::generic::UBoolProperty<UProperty>;
using UByteProperty = unreal::offsets::generic::UByteProperty<UProperty>;
using UDelegateProperty = unreal::offsets::generic::UDelegateProperty<UProperty>;
using UEnumProperty = unreal::offsets::generic::UEnumProperty<UProperty>;
using UFloatProperty = UProperty;
using UIntProperty = UProperty;
using UMulticastDelegateProperty = unreal::offsets::generic::UMulticastDelegateProperty<UProperty>;
using UObjectProperty = unreal::offsets::generic::UObjectProperty<UProperty>;
using UStructProperty = unreal::offsets::generic::UStructProperty<UProperty>;

using UByteAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<UByteProperty>;
using UClassProperty = unreal::offsets::generic::UClassProperty<UObjectProperty>;
using UFloatAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<UFloatProperty>;
using UIntAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<UIntProperty>;
using UInterfaceProperty = unreal::offsets::generic::UInterfaceProperty<UObjectProperty>;
using USoftClassProperty = unreal::offsets::generic::USoftClassProperty<UObjectProperty>;
;

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::game::tps

#endif

#endif /* UNREALSDK_GAME_TPS_OFFSETS_H */
