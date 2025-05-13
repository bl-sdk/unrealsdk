#ifndef UNREALSDK_GAME_BL3_OFFSETS_H
#define UNREALSDK_GAME_BL3_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/attribute_property.h"
#include "unrealsdk/unreal/classes/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
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

class UProperty : public UField {
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
    UProperty* PropertyLinkNext;

   private:
    UProperty* NextRef;
    UProperty* DestructorLinkNext;
    UProperty* PostConstructLinkNext;
};

class UStruct : public UField {
   public:
    UStruct* SuperField;
    UField* Children;

   private:
    int32_t PropertySize;
    int32_t MinAlignment;
    unreal::TArray<uint8_t> Script;

   public:
    UProperty* PropertyLink;

   private:
    UProperty* RefLink;
    UProperty* DestructorLink;
    UProperty* PostConstructLink;
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
    UProperty* FirstPropertyToInit;
    UFunction* EventGraphFunction;
    int32_t EventGraphCallOffset;
    void* Func;
};

using UConst = unreal::offsets::generic::UConst<UField>;

class UEnum : public UField {
    friend class unreal::UEnum;

   private:
    unreal::UnmanagedFString CppType;
    unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> Names;
    int64_t CppForm;
};

using UInt8Property = UProperty;
using UInt16Property = UProperty;
using UIntProperty = UProperty;
using UInt64Property = UProperty;
using UByteProperty = unreal::offsets::generic::UByteProperty<UProperty>;
using UUInt16Property = UProperty;
using UUInt32Property = UProperty;
using UUInt64Property = UProperty;
using UFloatProperty = UProperty;
using UDoubleProperty = UProperty;
using UNameProperty = UProperty;

using UByteAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<UByteProperty>;
using UFloatAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<UFloatProperty>;
using UIntAttributeProperty = unreal::offsets::generic::GenericAttributeProperty<UIntProperty>;

using UObjectProperty = unreal::offsets::generic::UObjectProperty<UProperty>;
using UClassProperty = unreal::offsets::generic::UClassProperty<UObjectProperty>;
using UInterfaceProperty = unreal::offsets::generic::UInterfaceProperty<UObjectProperty>;
using USoftClassProperty = unreal::offsets::generic::USoftClassProperty<UObjectProperty>;

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace unrealsdk::game::bl3

#endif

#endif /* UNREALSDK_GAME_BL3_OFFSETS_H */
