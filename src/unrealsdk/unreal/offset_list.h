#ifndef UNREALSDK_UNREAL_OFFSET_LIST_H
#define UNREALSDK_UNREAL_OFFSET_LIST_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/uobject.h"

namespace unrealsdk::unreal::offsets {

// Since this type is shared between dlls, also force consistent padding between compilers, even
// though this isn't an unreal type
#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UNREALSDK__DYNAMIC_OFFSET_TYPES(X) \
    X(UObject)                             \
    X(UField)

#define UNREALSDK_OFFSETS__DEFINE_OFFSET_LIST_MEMBERS(name) name::Offsets name;
#define UNREALSDK_OFFSETS__NESTED_FROM(name) name::Offsets::from<typename T::name>(),

// NOLINTEND(cppcoreguidelines-macro-usage)

struct OffsetList {
    // NOLINTNEXTLINE(readability-identifier-naming)
    UNREALSDK__DYNAMIC_OFFSET_TYPES(UNREALSDK_OFFSETS__DEFINE_OFFSET_LIST_MEMBERS)

    template <typename T>
    static constexpr OffsetList from(void) {
        return {UNREALSDK__DYNAMIC_OFFSET_TYPES(UNREALSDK_OFFSETS__NESTED_FROM)};
    }
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal::offsets

#endif /* UNREALSDK_UNREAL_OFFSET_LIST_H */
