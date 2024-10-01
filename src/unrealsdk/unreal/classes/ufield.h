#ifndef UNREALSDK_UNREAL_CLASSES_UFIELD_H
#define UNREALSDK_UNREAL_CLASSES_UFIELD_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uobject.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UField : public UObject {
   public:
    UField() = delete;
    UField(const UField&) = delete;
    UField(UField&&) = delete;
    UField& operator=(const UField&) = delete;
    UField& operator=(UField&&) = delete;
    ~UField() = delete;

#if !defined(UNREALSDK_GAME_BL1)

    // NOLINTNEXTLINE(readability-identifier-naming)
    UField* Next;

#else // defined(UNREALSDK_GAME_BL1)

   public:
    UStruct* SuperField;
    UField* Next;

#endif
};

template <>
struct ClassTraits<UField> {
    static inline const wchar_t* const NAME = L"Field";
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UFIELD_H */
