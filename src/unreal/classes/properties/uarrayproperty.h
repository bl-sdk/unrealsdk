#ifndef UNREAL_CLASSES_PROPERTIES_UARRAYPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_UARRAYPROPERTY_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/prop_traits.h"
#include "unreal/wrappers/wrapped_array.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UArrayProperty : public UProperty {
   private:
    friend PropTraits<UArrayProperty>;

    // NOLINTNEXTLINE(readability-identifier-naming)
    UProperty* Inner;
};

template <>
struct PropTraits<UArrayProperty> {
    using Value = WrappedArray;
    static inline const wchar_t* const CLASS = L"ArrayProperty";

    static Value get(const UArrayProperty* prop, uintptr_t addr);
    static void set(const UArrayProperty* prop, uintptr_t addr, Value value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_UARRAYPROPERTY_H */
