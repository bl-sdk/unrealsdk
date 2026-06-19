#ifndef UNREALSDK_GAME_BL4_OFFSETS_H
#define UNREALSDK_GAME_BL4_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/offsets_generic.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/gnames.h"
#include "unrealsdk/unreal/structs/tpair.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2

namespace unrealsdk::unreal {

struct FImplementedInterface;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl4 {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

class UClass;
class ZProperty;
struct FField;

using FFieldClass = unreal::offsets::generic::FFieldClass;                // Validated
struct FField : public unreal::offsets::generic::FField<FFieldClass> {};  // Validated

class UObject {  // Validated
   private:
    uintptr_t* vftable;

   public:
    int32_t ObjectFlags;
    int32_t InternalIndex;
    UClass* Class;
    unreal::FName Name;
    UObject* Outer;
};

using UField = unreal::offsets::generic::UField<UObject>;  // validated

class UStruct : public UField {
   private:
    uint8_t UnknownData00[0x10];

   public:
    UStruct* SuperField;
    UField* Children;
    FField* ChildProperties;
    int32_t PropertySize;
    int32_t MinAlignment;

   private:
    unreal::TArray<uint8_t> Script;

   public:
    ZProperty* PropertyLink;

   private:  // Validated up to this point
    ZProperty* RefLink;
    ZProperty* DestructorLink;
    ZProperty* PostConstructLink;
    unreal::TArray<UObject*> ScriptObjectReferences;
};

class UClass : public UStruct {
   private:
    uint8_t UnknownData00[0x70];

   public:
    UObject* ClassDefaultObject;  // Validated

   private:
    uint8_t UnknownData01[0xC0];

   public:
    unreal::TArray<unreal::FImplementedInterface> Interfaces;  // Validated
};

using UScriptStruct = unreal::offsets::generic::UScriptStruct<UStruct>;

class UFunction : public UStruct {
   private:
    char _0x00[0x10];

   public:
    uint32_t FunctionFlags;
    uint8_t NumParams;  // Validated
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

class ZProperty : public FField {  // Validated
   public:
    int32_t ArrayDim;
    int32_t ElementSize;
    uint64_t PropertyFlags;

   private:
    uint16_t RepIndex;
    uint8_t BlueprintReplicationCondition;
    uint8_t UnknownData00[1];

   public:
    int32_t Offset_Internal;

   public:
    ZProperty* PropertyLinkNext;

   private:
    ZProperty* NextRef;
    ZProperty* DestructorLinkNext;
    ZProperty* PostConstructLinkNext;
    unreal::FName RepNotifyFunc;
};

class ZArrayProperty : public ZProperty {  // validated
   public:
    uint64_t ArrayFlags;
    ZProperty* Inner;
};
using ZByteProperty = unreal::offsets::generic::ZByteProperty<ZProperty>;
using ZDelegateProperty = unreal::offsets::generic::ZDelegateProperty<ZProperty>;
using ZEnumProperty = unreal::offsets::generic::ZEnumProperty<ZProperty>;
class ZFloatProperty : public ZProperty {};
using ZInterfaceProperty = unreal::offsets::generic::ZInterfaceProperty<ZProperty>;
class ZIntProperty : public ZProperty {};
using ZMulticastDelegateProperty = unreal::offsets::generic::ZMulticastDelegateProperty<ZProperty>;
using ZObjectProperty = unreal::offsets::generic::ZObjectProperty<ZProperty>;
using ZStructProperty = unreal::offsets::generic::ZStructProperty<ZProperty>;  // validated

using ZByteAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<ZByteProperty>;
using ZClassProperty = unreal::offsets::generic::ZClassProperty<ZObjectProperty>;
using ZFloatAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<ZFloatProperty>;
using ZIntAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<ZIntProperty>;
using ZSoftClassProperty = unreal::offsets::generic::ZSoftClassProperty<ZObjectProperty>;
using ZGameDataHandleProperty =
    unreal::offsets::generic::ZGameDataHandleProperty<ZProperty>;                    // Validated
using ZGbxDefPtrProperty = unreal::offsets::generic::ZGbxDefPtrProperty<ZProperty>;  // Validated
using ZGbxInlineStructProperty =
    unreal::offsets::generic::ZGbxInlineStructProperty<ZStructProperty>;  // Validated

class ZBoolProperty : public ZProperty {
   private:
    uint8_t FieldSize;
    uint8_t ByteOffset;
    uint8_t ByteMask;

   public:
    uint8_t FieldMask;
};

struct FNameEntry {
    static const constexpr auto META_SIZE_BIT_OFFSET = 6;

    union {
        uint8_t Flags;
        uint16_t Metadata;
    };
    unreal::FNameEntry::name_union Name;
};  // Validated

struct FFrame {
   private:
    void* VfTable;
    uint32_t Unknown00;

   public:
    UFunction* Node;  // Validated
    UObject* Object;  // Validated
    uint8_t* Code;    // Validated

   private:
    void* Locals;
    ZProperty* LastProperty;  // Validated
    void* LastPropertyAddress;
    FFrame* PreviousFrame;
    void* OutParams;
};

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace unrealsdk::game::bl4

#endif

#endif /* UNREALSDK_GAME_BL4_OFFSETS_H */
