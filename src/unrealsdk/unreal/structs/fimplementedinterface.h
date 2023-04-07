#ifndef UNREAL_STRUCTS_FIMPLEMENTEDINTERFACE_H
#define UNREAL_STRUCTS_FIMPLEMENTEDINTERFACE_H

#include "pch.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UStructProperty;
class UClass;

struct FImplementedInterface {
    // NOLINTBEGIN(readability-identifier-naming)

    UClass* Class;

   private:
#ifdef UE4
    int32_t PointerOffset;
    bool isNative;
#else
    UStructProperty* VFTableProperty;  // May be null (if native?)
#endif

    // NOLINTEND(readability-identifier-naming)

   public:
    /**
     * @brief Gets the offset of the interface pointer on this class.
     *
     * @return The interface pointer offset.
     */
    [[nodiscard]] size_t get_pointer_offset(void) const;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_FIMPLEMENTEDINTERFACE_H */
