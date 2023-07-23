#ifndef UNREALSDK_UNREAL_STRUCTS_FTEXT_H
#define UNREALSDK_UNREAL_STRUCTS_FTEXT_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/tsharedpointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

struct FTextData {
    uintptr_t* vftable;
};

struct FText {
    static const constexpr auto FLAG_TRANSIENT = 1 << 0;
    static const constexpr auto FLAG_INVARIANT_CULTURE = 1 << 1;
    static const constexpr auto FLAG_FROM_NAME_OR_STRING = 1 << 4;

    TSharedPointer<FTextData> data;
    uint32_t flags;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FTEXT_H */
