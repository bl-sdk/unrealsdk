#ifndef UNREAL_CLASSES_UFIELD_H
#define UNREAL_CLASSES_UFIELD_H

#include "unreal/classes/uobject.h"

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

    // NOLINTNEXTLINE(readability-identifier-naming)
    UField* Next;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UFIELD_H */
