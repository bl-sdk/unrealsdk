#ifndef UNREAL_CLASSES_USCRIPTSTRUCT_H
#define UNREAL_CLASSES_USCRIPTSTRUCT_H

#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UScriptStruct : public UStruct {
   public:
    UScriptStruct() = delete;
    UScriptStruct(const UScriptStruct&) = delete;
    UScriptStruct(UScriptStruct&&) = delete;
    UScriptStruct& operator=(const UScriptStruct&) = delete;
    UScriptStruct& operator=(UScriptStruct&&) = delete;
    ~UScriptStruct() = delete;

    // NOLINTNEXTLINE(readability-identifier-naming)
    uint32_t StructFlags;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_USCRIPTSTRUCT_H */
