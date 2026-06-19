#ifndef UNREALSDK_GAME_BL1E_OFFSETS_H
#define UNREALSDK_GAME_BL1E_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl1/offsets.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/unreal//properties/attribute_property.h"
#include "unrealsdk/unreal//properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal//properties/zarrayproperty.h"
#include "unrealsdk/unreal//properties/zbyteproperty.h"
#include "unrealsdk/unreal//properties/zclassproperty.h"
#include "unrealsdk/unreal//properties/zdelegateproperty.h"
#include "unrealsdk/unreal//properties/zenumproperty.h"
#include "unrealsdk/unreal//properties/zinterfaceproperty.h"
#include "unrealsdk/unreal//properties/zmulticastdelegateproperty.h"
#include "unrealsdk/unreal//properties/zobjectproperty.h"
#include "unrealsdk/unreal//properties/zstructproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/offsets.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

namespace unrealsdk::unreal {
struct FImplementedInterface;
}

namespace unrealsdk::game::bl1e {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UClass;
class UStruct;

using UObject = bl2::generic::UObject<UClass>;

// Size=104
class UField : public UObject {
   public:
    UField* Next;
};

// Size=184b
class ZProperty : public UField {
   public:
    int32_t ArrayDim;        // 104b
    int32_t ElementSize;     // 108b
    uint32_t PropertyFlags;  // 112b
    uint8_t UnknownData00[0x18];
    int32_t Offset_Internal;      // 140b
    ZProperty* PropertyLinkNext;  // 144b
    uint8_t UnknownData01[0x20];
};

// Size=208
class UStruct : public UField {
   public:
    uint8_t UnknownData00[0x10];
    UStruct* SuperField;    // 120b
    UField* Children;       // 128b
    uint16_t PropertySize;  // 136b
    uint8_t UnknownData01[0x26];
    ZProperty* PropertyLink;  // 176b
    uint8_t UnknownData02[0x8];
    unreal::TArray<UObject*> ScriptObjectReferences;  // 192b
};

// Size=1000 ish ( atleast based on struct size for Core.Class )
class UClass : public UStruct {
   public:
    uint8_t UnknownData00[0x104];
    UObject* ClassDefaultObject;  // 468b
    uint8_t UnknownData01[0x60];
    unreal::TArray<unreal::FImplementedInterface> Interfaces;  // 572b
};

using UScriptStruct = unreal::offsets::generic::UScriptStruct<UStruct>;
using UFunction = bl2::generic::UFunction<UStruct>;
using UConst = unreal::offsets::generic::UConst<UField>;
using UEnum = bl2::generic::UEnum<UField>;

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

struct FFieldClass : public unreal::offsets::generic::FFieldClass {};
struct FField : public unreal::offsets::generic::FField<FFieldClass> {};

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::game::bl1e

#endif

#endif /* UNREALSDK_GAME_BL1_OFFSETS_H */
