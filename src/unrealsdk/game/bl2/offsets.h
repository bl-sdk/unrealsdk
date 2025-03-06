#ifndef UNREALSDK_GAME_BL2_OFFSETS_H
#define UNREALSDK_GAME_BL2_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/tarray.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UClass;

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

struct UObject {
   private:
    uintptr_t* vftable;
    void* HashNext;

   public:
    uint64_t ObjectFlags;

   private:
    void* HashOuterNext;
    void* StateFrame;
    UObject* _Linker;
    void* _LinkerIndex;

   public:
    int32_t InternalIndex;

   private:
    int32_t NetIndex;

   public:
    UObject* Outer;
    unreal::FName Name;
    unreal::UClass* Class;  // TODO

   private:
    UObject* ObjectArchetype;
};

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
