#ifndef UNREAL_CLASSES_UPROPERTY_H
#define UNREAL_CLASSES_UPROPERTY_H

#include "unreal/classes/ufield.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UProperty : public UField {
   public:
    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

#ifdef UE4
    int32_t ArrayDim;
    int32_t ElementSize;
    uint64_t PropertyFlags;

   private:
    uint16_t RepIndex;
    uint8_t BlueprintReplicationCondition;

   public:
    int32_t Offset_Internal;

   private:
    FName RepNotifyFunc;

   public:
    /** In memory only: Linked list of properties from most-derived to base **/
    UProperty* PropertyLinkNext;

   private:
    /** In memory only: Linked list of object reference properties from most-derived to base **/
    UProperty* NextRef;
    /** In memory only: Linked list of properties requiring destruction. Note this does not include
     * things that will be destroyed byt he native destructor **/
    UProperty* DestructorLinkNext;
    /** In memory only: Linked list of properties requiring post constructor initialization.**/
    UProperty* PostConstructLinkNext;  // 0x0030(0x0040) MISSED OFFSET
#else

    int32_t ArrayDim;
    int32_t ElementSize;
    uint32_t PropertyFlags;

   private:
    uint8_t UnknownData00[0x14];

   public:
    int32_t Offset_Internal;
    UProperty* PropertyLinkNext;

   private:
    uint8_t UnknownData01[0x18];
#endif

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UPROPERTY_H */
