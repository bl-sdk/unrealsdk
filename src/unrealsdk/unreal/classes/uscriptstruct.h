#ifndef UNREALSDK_UNREAL_CLASSES_USCRIPTSTRUCT_H
#define UNREALSDK_UNREAL_CLASSES_USCRIPTSTRUCT_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

namespace offsets::generic {

template <typename T>
class UScriptStruct : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    uint32_t StructFlags;
};

}  // namespace offsets::generic

class UScriptStruct : public UStruct {
   public:
    UScriptStruct() = delete;
    UScriptStruct(const UScriptStruct&) = delete;
    UScriptStruct(UScriptStruct&&) = delete;
    UScriptStruct& operator=(const UScriptStruct&) = delete;
    UScriptStruct& operator=(UScriptStruct&&) = delete;
    ~UScriptStruct() = delete;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_USCRIPTSTRUCT_FIELDS(X) X(uint32_t, StructFlags)

    // NOLINTNEXTLINE(readability-identifier-naming)
    UNREALSDK_DEFINE_FIELDS_HEADER(UScriptStruct, UNREALSDK_USCRIPTSTRUCT_FIELDS);

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    uint32_t StructFlags_internal;
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
