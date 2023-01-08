#ifndef UNREAL_CLASSES_USTRUCT_H
#define UNREAL_CLASSES_USTRUCT_H

#include "unreal/classes/ufield.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif
#endif

class UStruct : public UField {
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_USTRUCT_H */
