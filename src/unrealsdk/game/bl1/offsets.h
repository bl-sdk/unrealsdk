#ifndef UNREALSDK_GAME_BL1_OFFSETS_H
#define UNREALSDK_GAME_BL1_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/offsets_generic.h"
#include "unrealsdk/unreal/properties/attribute_property.h"
#include "unrealsdk/unreal/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/properties/zarrayproperty.h"
#include "unrealsdk/unreal/properties/zbyteproperty.h"
#include "unrealsdk/unreal/properties/zclassproperty.h"
#include "unrealsdk/unreal/properties/zdelegateproperty.h"
#include "unrealsdk/unreal/properties/zenumproperty.h"
#include "unrealsdk/unreal/properties/zinterfaceproperty.h"
#include "unrealsdk/unreal/properties/zmulticastdelegateproperty.h"
#include "unrealsdk/unreal/properties/zobjectproperty.h"
#include "unrealsdk/unreal/properties/zstructproperty.h"
#include "unrealsdk/unreal/structs/fname.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

namespace unrealsdk::unreal {

struct FImplementedInterface;

}

namespace unrealsdk::game::bl1 {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UStruct;
class UClass;

class UObject {
   private:
    uintptr_t* vftable;

   public:
    int32_t InternalIndex;
    uint64_t ObjectFlags;

   private:
    void* HashNext;
    void* HashOuterNext;
    void* StateFrame;
    UObject* _Linker;
    void* _LinkerIndex;
    int32_t NetIndex;

   public:
    UObject* Outer;
    unreal::FName Name;
    UClass* Class;
    UObject* ObjectArchetype;
};

class UField : public UObject {
   public:
    // Superfield is considered to be part of UStruct in the rest of the sdk
    // Luckily, we can just expose it here and it'll all get picked up right
    UStruct* SuperField;
    UField* Next;
};

class ZProperty : public UField {
   public:
    int32_t ArrayDim;
    int32_t ElementSize;
    uint32_t PropertyFlags;

   private:
    uint8_t UnknownData00[0x14];

   public:
    int32_t Offset_Internal;
    ZProperty* PropertyLinkNext;

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
    ZProperty* PropertyLink;

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

}  // namespace unrealsdk::game::bl1

#endif

#endif /* UNREALSDK_GAME_BL1_OFFSETS_H */
