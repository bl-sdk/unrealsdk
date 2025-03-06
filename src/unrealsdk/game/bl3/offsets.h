#ifndef UNREALSDK_GAME_BL3_OFFSETS_H
#define UNREALSDK_GAME_BL3_OFFSETS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/tarray.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::unreal {

class UClass;  // TODO

}  // namespace unrealsdk::unreal

namespace unrealsdk::game::bl3 {

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

   public:
    int32_t ObjectFlags;
    int32_t InternalIndex;
    unreal::UClass* Class;  // TODO
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

class UStruct : public bl3::UField {
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

// NOLINTEND(cppcoreguidelines-pro-type-member-init,
//           readability-identifier-naming,
//           readability-magic-numbers)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace unrealsdk::game::bl3

#endif

#endif /* UNREALSDK_GAME_BL3_OFFSETS_H */
