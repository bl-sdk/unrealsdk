#ifndef UNREAL_CLASSES_UCLASS_H
#define UNREAL_CLASSES_UCLASS_H

#include "unreal/classes/ustruct.h"
#include "unreal/structs/fimplementedinterface.h"
#include "unreal/structs/tarray.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UClass : public UStruct {
   public:
    UClass() = delete;
    UClass(const UClass&) = delete;
    UClass(UClass&&) = delete;
    UClass& operator=(const UClass&) = delete;
    UClass& operator=(UClass&&) = delete;
    ~UClass() = delete;

    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

#ifdef UE4
   private:
    uint8_t UnknownData00[0x70];

   public:
    UObject* ClassDefaultObject;

   private:
    uint8_t UnknownData01[0xA0];

   public:
    TArray<FImplementedInterface> Interfaces;
#else
   private:
    // Misc Fields I found within this block in BL2, but which we don't care about enough for me to
    //  find in UE4, or to want to increase the compile times by including

    // 0xE8: TArray<FRepRecord> ClassReps;
    // 0xF4: TArray<UField*> NetFields;
    // 0x100: TArray<FName> HideCategories;
    // 0x10C: TArray<FName> AutoExpandCategories;

    uint8_t UnknownData00[0xCC];

   public:
    UObject* ClassDefaultObject;

   private:
    uint8_t UnknownData01[0x48];

   public:
    TArray<FImplementedInterface> Interfaces;
#endif

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UCLASS_H */
