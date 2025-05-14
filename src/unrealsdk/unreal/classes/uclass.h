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

    UNREALSDK_DEFINE_FIELDS_HEADER(UClass, UNREALSDK_UCLASS_FIELDS);

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
