#ifndef UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/prop_traits.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UStrProperty : public UProperty {};

template <>
struct PropTraits<UStrProperty> {
    using Value = std::wstring;
    static inline const wchar_t* const CLASS = L"StrProperty";

    static Value get(UStrProperty* prop, uintptr_t addr);
    static void set(UStrProperty* prop, uintptr_t addr, Value value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H */
