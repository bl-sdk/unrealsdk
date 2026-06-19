#ifndef UNREALSDK_UNREAL_OFFSETS_GENERIC_H
#define UNREALSDK_UNREAL_OFFSETS_GENERIC_H

#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"

// This file contains a number of templates for simpler unreal types, which you can use to define
// offsets in a one-liner:
//     using UField = unreal::offsets::generic::UField<UObject>;

namespace unrealsdk::unreal {

class ZArrayProperty;
class UClass;
class UEnum;
class UFunction;
class UObject;
class ZProperty;
class UScriptStruct;
template <typename FFieldType, typename UObjectType>
struct TFieldVariant;

namespace offsets::generic {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,
//             readability-identifier-naming,
//             readability-magic-numbers)

template <typename T>
class GenericAttributeProperty : public T {
   public:
    unreal::ZArrayProperty* ModifierStackProperty;
    GenericAttributeProperty<T>* OtherAttributeProperty;
};

template <typename T>
class ZArrayProperty : public T {
   public:
    unreal::ZProperty* Inner;
};

template <typename T>
class ZByteProperty : public T {
   public:
    unreal::UEnum* Enum;
};

template <typename T>
class ZClassProperty : public T {
   public:
    unreal::UClass* MetaClass;
};

template <typename T>
class UConst : public T {
   public:
    UnmanagedFString Value;
};

template <typename T>
class ZDelegateProperty : public T {
   public:
    unreal::UFunction* Signature;
};

template <typename T>
class ZEnumProperty : public T {
   public:
    unreal::ZProperty* UnderlyingProp;
    unreal::UEnum* Enum;
};

template <typename T>
class UField : public T {
   public:
    UField* Next;
};

template <typename T>
class ZGameDataHandleProperty : public T {
   public:
    uint32_t TypeHandle;
};

template <typename T>
class ZInterfaceProperty : public T {
   public:
    unreal::UClass* InterfaceClass;
};

template <typename T>
class ZMulticastDelegateProperty : public T {
   public:
    unreal::UFunction* Signature;
};

template <typename T>
class ZObjectProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    unreal::UClass* PropertyClass;
};

template <typename T>
class UScriptStruct : public T {
   public:
    uint32_t StructFlags;
};

template <typename T>
class ZSoftClassProperty : public T {
   public:
    unreal::UClass* MetaClass;
};

template <typename T>
class ZStructProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    unreal::UScriptStruct* Struct;
};

template <typename T>
struct FField {
   private:
    uintptr_t* vftable;

   public:
    T* Class;
    TFieldVariant<FField, UObject>* Owner;
    FField* Next;
    unreal::FName Name;

   private:
    uint64_t FlagsPrivate;
};

struct FFieldClass {
   public:
    unreal::FName Name;

   private:
    // Should this really go in generic while it has unknown data... eh
    uint8_t UnknownData[0x28];

   public:
    FFieldClass* SuperField;
};

template <typename T>
class ZGbxDefPtrProperty : public T {
   public:
    unreal::UScriptStruct* Struct;
};

template <typename T>
class ZGbxInlineStructProperty : public T {
   public:
    unreal::UScriptStruct* MetaStruct;
};

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace offsets::generic

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_OFFSETS_GENERIC_H */
