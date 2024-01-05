#ifndef UNREALSDK_UNREAL_CLASSES_UENUM_H
#define UNREALSDK_UNREAL_CLASSES_UENUM_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tpair.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

class UEnum : public UField {
   public:
    UEnum() = delete;
    UEnum(const UEnum&) = delete;
    UEnum(UEnum&&) = delete;
    UEnum& operator=(const UEnum&) = delete;
    UEnum& operator=(UEnum&&) = delete;
    ~UEnum() = delete;

   private:
    // NOLINTBEGIN(readability-identifier-naming)

#ifdef UE4
    UnmanagedFString CppType;
    TArray<TPair<FName, uint64_t>> Names;
    int64_t CppForm;
#endif

    // NOLINTEND(readability-identifier-naming)

   public:
    /**
     * @brief Get the enum values which have assigned names.
     *
     * @return A map of integer values to their associated name.
     */
    [[nodiscard]] std::unordered_map<FName, uint64_t> get_names(void) const;
};

template <>
struct ClassTraits<UEnum> {
    static inline const wchar_t* const NAME = L"Enum";
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UENUM_H */
