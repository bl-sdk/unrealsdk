#ifndef UNREALSDK_GAME_BL2_OFFSETS_H
#define UNREALSDK_GAME_BL2_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/tarray.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

struct FImplementedInterface;

}

namespace unrealsdk::game::bl2 {

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

class UClass;
class UProperty;

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

}  // namespace generic

using UObject = bl2::generic::UObject<UClass>;
using UField = unreal::offsets::generic::UField<UObject>;

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
    uint8_t UnknownData01[0x18];
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

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::game::bl2

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_H */
