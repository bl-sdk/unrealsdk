#ifndef UNREAL_CLASSES_UCLASS_H
#define UNREAL_CLASSES_UCLASS_H

#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif
#endif

class UClass : public UStruct {
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UCLASS_H */
