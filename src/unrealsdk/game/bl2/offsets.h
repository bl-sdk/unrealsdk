#ifndef UNREALSDK_GAME_BL2_OFFSETS_H
#define UNREALSDK_GAME_BL2_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"
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
#include "unrealsdk/unreal/structs/gnames.h"
#include "unrealsdk/unreal/structs/tarray.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

namespace unrealsdk::unreal {

struct FImplementedInterface;

}

namespace unrealsdk::game::bl2 {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UClass;
class ZProperty;

namespace generic {

// This is generic just so that we can reuse it in bl1+tps, but swap out the type of Class
template <typename UClass>
class UObject {
   private:
    uintptr_t* vftable;
    void* HashNext;

   public:
    uint64_t ObjectFlags;

   private:
    void* HashOuterNext;
    void* StateFrame;
    UObject<UClass>* _Linker;
    void* _LinkerIndex;

   public:
    int32_t InternalIndex;

   private:
    int32_t NetIndex;

   public:
    UObject<UClass>* Outer;
    unreal::FName Name;
    UClass* Class;

   private:
    UObject<UClass>* ObjectArchetype;
};

template <typename T>
class UFunction : public T {
   public:
    uint32_t FunctionFlags;

   private:
    uint16_t iNative;
    uint16_t RepOffset;
    unreal::FName FriendlyName;
    uint8_t OperPrecedence;

   public:
    uint8_t NumParams;
    uint16_t ParamsSize;
    uint16_t ReturnValueOffset;

   private:
    uint8_t UnknownData00[0x6];
    void* Func;
};

template <typename T>
class UEnum : public T {
   public:
    unreal::TArray<unreal::FName> Names;
};

template <typename T>
class ZBoolProperty : public T {
   public:
    uint32_t FieldMask;
};

}  // namespace generic

using UObject = bl2::generic::UObject<UClass>;
using UField = unreal::offsets::generic::UField<UObject>;

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
    uint8_t UnknownData01[0x18];
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
    uint8_t UnknownData02[0x10];

    unreal::TArray<UObject*> ScriptObjectReferences;
};

class UClass : public UStruct {
    // Misc Fields I found within this block in BL2, but which we really don't care about

    // 0xE8: TArray<FRepRecord> ClassReps;
    // 0xF4: TArray<UField*> NetFields;
    // 0x100: TArray<FName> HideCategories;
    // 0x10C: TArray<FName> AutoExpandCategories;

   private:
    uint8_t UnknownData00[0xCC];

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

struct FNameEntry {
   private:
    uint8_t UnknownData00[0x08];

   public:
    union {
        uint8_t Flags;
        int32_t Index;
    };

   private:
    void* UnknownData01;

   public:
    unreal::FNameEntry::name_union Name;
};

struct FFrame {
   private:
    void* VfTable;
    uint32_t bAllowSuppression;
    uint32_t bSuppressEventTag;
    uint32_t bAutoEmitLineTerminator;

   public:
    UFunction* Node;
    UObject* Object;
    uint8_t* Code;

   private:
    void* Locals;

   public:
    FFrame* PreviousFrame;

   private:
    void* OutParams;
};

struct FFieldClass : public unreal::offsets::generic::FFieldClass {};
struct FField : public unreal::offsets::generic::FField<FFieldClass> {};

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::game::bl2

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_H */
