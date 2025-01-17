#ifndef UNREALSDK_UNREAL_CLASSES_USCRIPTSTRUCT_H
#define UNREALSDK_UNREAL_CLASSES_USCRIPTSTRUCT_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ustruct.h"

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

    // NOLINTBEGIN(readability-identifier-naming)

   private:
    uint32_t StructFlags_internal;

   public:
    decltype(StructFlags_internal)& StructFlags(void);
    [[nodiscard]] const decltype(StructFlags_internal)& StructFlags(void) const;

    // NOLINTEND(readability-identifier-naming)
};

template <>
struct ClassTraits<UScriptStruct> {
    static inline const wchar_t* const NAME = L"ScriptStruct";
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_USCRIPTSTRUCT_H */
