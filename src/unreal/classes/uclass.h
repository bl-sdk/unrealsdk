#ifndef UNREAL_CLASSES_UCLASS_H
#define UNREAL_CLASSES_UCLASS_H

#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UClass : public UStruct {
   public:
    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

#ifdef UE4
   private:
    uint8_t UnknownData00[0x70];

   public:
    UObject* ClassDefaultObject;
#else
   private:
    uint8_t UnknownData00[0xCC];

   public:
    UObject* ClassDefaultObject;
#endif

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UCLASS_H */
