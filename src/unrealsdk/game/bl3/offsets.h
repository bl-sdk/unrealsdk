#ifndef UNREALSDK_GAME_BL3_OFFSETS_H
#define UNREALSDK_GAME_BL3_OFFSETS_H

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
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/gnames.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tpair.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

namespace unrealsdk::unreal {

struct FImplementedInterface;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl3 {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UClass;
struct FField;

class UObject {
   private:
    uintptr_t* vftable;

   public:
    int32_t ObjectFlags;
    int32_t InternalIndex;
    UClass* Class;
    unreal::FName Name;
    UObject* Outer;
};

using UField = unreal::offsets::generic::UField<UObject>;

class ZProperty : public UField {
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
    ZProperty* PropertyLinkNext;

   private:
    ZProperty* NextRef;
    ZProperty* DestructorLinkNext;
    ZProperty* PostConstructLinkNext;
};

class UStruct : public UField {
    friend class unreal::UStruct;

   public:
    UStruct* SuperField;
    UField* Children;
    int32_t PropertySize;

   private:
    int32_t MinAlignment;
    unreal::TArray<uint8_t> Script;

   public:
    ZProperty* PropertyLink;

   private:
    ZProperty* RefLink;
    ZProperty* DestructorLink;
    ZProperty* PostConstructLink;
    unreal::TArray<UObject*> ScriptObjectReferences;
};

class UClass : public UStruct {
   private:
    uint8_t UnknownData00[0x70];

   public:
    UObject* ClassDefaultObject;

   private:
    uint8_t UnknownData01[0xA0];

   public:
    unreal::TArray<unreal::FImplementedInterface> Interfaces;
};

using UScriptStruct = unreal::offsets::generic::UScriptStruct<UStruct>;

class UFunction : public UStruct {
   public:
    uint32_t FunctionFlags;
    uint8_t NumParams;
    uint16_t ParamsSize;
    uint16_t ReturnValueOffset;

   private:
    uint16_t RPCId;
    uint16_t RPCResponseId;
    ZProperty* FirstPropertyToInit;
    UFunction* EventGraphFunction;
    int32_t EventGraphCallOffset;
    void* Func;
};

using UConst = unreal::offsets::generic::UConst<UField>;

class UEnum : public UField {
   private:
    unreal::UnmanagedFString CppType;

   public:
    unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> Names;

   private:
    int64_t CppForm;
};

using ZArrayProperty = unreal::offsets::generic::ZArrayProperty<ZProperty>;
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

class ZBoolProperty : public ZProperty {
   private:
    uint8_t FieldSize;
    uint8_t ByteOffset;
    uint8_t ByteMask;

   public:
    uint8_t FieldMask;
};

struct FNameEntry {
   public:
    union {
        uint8_t Flags;
        int32_t Index;
    };

   private:
    uint8_t UnknownData00[0x04];
    FNameEntry* HashNext;

   public:
    unreal::FNameEntry::name_union Name;
};

struct FFrame {
   private:
    void* VfTable;
    uint32_t bSuppressEventTag;
    uint32_t bAutoEmitLineTerminator;

   public:
    UFunction* Node;
    UObject* Object;
    uint8_t* Code;

   private:
    void* Locals;
    ZProperty* LastProperty;
    void* LastPropertyAddress;

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

}  // namespace unrealsdk::game::bl3

#endif

#endif /* UNREALSDK_GAME_BL3_OFFSETS_H */
