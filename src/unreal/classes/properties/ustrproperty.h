#ifndef UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/prop_traits.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UStrProperty : public UProperty {
   public:
    UStrProperty() = delete;
    UStrProperty(const UStrProperty&) = delete;
    UStrProperty(UStrProperty&&) = delete;
    UStrProperty& operator=(const UStrProperty&) = delete;
    UStrProperty& operator=(UStrProperty&&) = delete;
    ~UStrProperty() = delete;
};

template <>
struct PropTraits<UStrProperty> {
    using Value = std::wstring;
    static inline const wchar_t* const CLASS = L"StrProperty";

    static Value get(const UStrProperty* prop, uintptr_t addr);
    static void set(const UStrProperty* prop, uintptr_t addr, const Value& value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H */
