#ifndef UNREALSDK_UNREAL_CLASSES_UCLASS_H
#define UNREALSDK_UNREAL_CLASSES_UCLASS_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fimplementedinterface.h"
#include "unrealsdk/unreal/structs/tarray.h"

namespace unrealsdk::unreal {

struct FImplementedInterface;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
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

#if !defined(UNREALSDK_GAME_BL1)

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

#else // defined(UNREALSDK_GAME_BL1)

    // NOTE: I don't know if the class has changed size; Will do a scan in ghidra for 1AC

   public:
    uint8_t UnknownData00[0xC0];
    UObject* ClassDefaultObject; // 340b
    uint8_t UnknownData01[0x48];
    TArray<FImplementedInterface> Interfaces; //  416b

#endif

#endif

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)

    /**
     * @brief Checks if this class implements an interface.
     *
     * @param iface The interface to check.
     * @param[out] impl_out If not null, gets set to the interface implementation for this object
     *                      (assuming it's found).
     * @return True if this class implements the given interface.
     */
    [[nodiscard]] bool implements(const UClass* iface,
                                  FImplementedInterface* impl_out = nullptr) const;
};

template <>
struct ClassTraits<UClass> {
    static inline const wchar_t* const NAME = L"Class";
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UCLASS_H */
