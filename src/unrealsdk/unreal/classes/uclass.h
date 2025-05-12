#ifndef UNREALSDK_UNREAL_CLASSES_UCLASS_H
#define UNREALSDK_UNREAL_CLASSES_UCLASS_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fimplementedinterface.h"
#include "unrealsdk/unreal/structs/tarray.h"

namespace unrealsdk::unreal {

struct FImplementedInterface;

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
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

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UCLASS_FIELDS(X)  \
    X(UObject*, ClassDefaultObject) \
    X(TArray<FImplementedInterface>, Interfaces)

    // NOLINTNEXTLINE(readability-identifier-naming)
    UNREALSDK_DEFINE_FIELDS_HEADER(UClass, UNREALSDK_UCLASS_FIELDS);

    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

   private:
#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    uint8_t UnknownData00[0x70];
    UObject* ClassDefaultObject_internal;
    uint8_t UnknownData01[0xA0];
    TArray<FImplementedInterface> Interfaces_internal;
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    // Misc Fields I found within this block in BL2, but which we don't care about enough for me to
    //  find in UE4, or to want to increase the compile times by including

    // 0xE8: TArray<FRepRecord> ClassReps;
    // 0xF4: TArray<UField*> NetFields;
    // 0x100: TArray<FName> HideCategories;
    // 0x10C: TArray<FName> AutoExpandCategories;

    uint8_t UnknownData00[0xCC];
    UObject* ClassDefaultObject_internal;
    uint8_t UnknownData01[0x48];
    TArray<FImplementedInterface> Interfaces_internal;

#else
#error Unknown SDK flavour
#endif
    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
   public:
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

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UCLASS_H */
