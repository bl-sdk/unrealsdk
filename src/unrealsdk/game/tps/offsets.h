#ifndef UNREALSDK_GAME_TPS_OFFSETS_H
#define UNREALSDK_GAME_TPS_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/offsets_generic.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

namespace unrealsdk::game::tps {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UClass;

using UObject = bl2::generic::UObject<UClass>;
class UField : public bl2::UField {};

class ZProperty : public UField {
   public:
    int32_t ArrayDim;
    int32_t ElementSize;
    uint32_t PropertyFlags;

   private:
    uint8_t UnknownData00[0x14];

   public:
    int32_t Offset_Internal;
    unreal::ZProperty* PropertyLinkNext;

   private:
    uint8_t UnknownData01[0xC];
};

class UStruct : public UField {
   private:
    uint8_t UnknownData00[0x8];

   public:
    UStruct* SuperField;
    UField* Children;
    uint16_t PropertySize;

   private:
    uint8_t UnknownData01[0x1A];

   public:
    ZProperty* PropertyLink;

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
    uint8_t UnknownData01[0x10];

   public:
    unreal::TArray<unreal::FImplementedInterface> Interfaces;
};

using UScriptStruct = unreal::offsets::generic::UScriptStruct<UStruct>;
using UFunction = bl2::generic::UFunction<UStruct>;
class UConst : public bl2::UConst {};
class UEnum : public bl2::UEnum {};

using ZArrayProperty = unreal::offsets::generic::ZArrayProperty<ZProperty>;
using ZBoolProperty = bl2::generic::ZBoolProperty<ZProperty>;
using ZByteProperty = unreal::offsets::generic::ZByteProperty<ZProperty>;
using ZDelegateProperty = unreal::offsets::generic::ZDelegateProperty<ZProperty>;
using ZEnumProperty = unreal::offsets::generic::ZEnumProperty<ZProperty>;
class ZFloatProperty : public ZProperty {};
using ZInterfaceProperty = unreal::offsets::generic::ZInterfaceProperty<ZProperty>;
class ZIntProperty : public ZProperty {};
using ZMulticastDelegateProperty = unreal::offsets::generic::ZMulticastDelegateProperty<ZProperty>;
using ZObjectProperty = unreal::offsets::generic::ZObjectProperty<ZProperty>;
using ZStructProperty = unreal::offsets::generic::ZStructProperty<ZProperty>;

using ZByteAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<ZByteProperty>;
using ZClassProperty = unreal::offsets::generic::ZClassProperty<ZObjectProperty>;
using ZFloatAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<ZFloatProperty>;
using ZIntAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<ZIntProperty>;
using ZSoftClassProperty = unreal::offsets::generic::ZSoftClassProperty<ZObjectProperty>;
using ZGameDataHandleProperty = unreal::offsets::generic::ZGameDataHandleProperty<ZProperty>;
using ZGbxDefPtrProperty = unreal::offsets::generic::ZGbxDefPtrProperty<ZProperty>;
using ZGbxInlineStructProperty =
    unreal::offsets::generic::ZGbxInlineStructProperty<ZStructProperty>;

class FNameEntry : public bl2::FNameEntry {};
class FFrame : public bl2::FFrame {};
class FField : public bl2::FField {};
class FFieldClass : public bl2::FFieldClass {};

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::game::tps

#endif

#endif /* UNREALSDK_GAME_TPS_OFFSETS_H */
