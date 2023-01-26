#ifndef UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/prop_traits.h"
#include "unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

class UScriptStruct;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UStructProperty : public UProperty {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UScriptStruct* Struct;
};

template <>
struct PropTraits<UStructProperty> {
    using Value = WrappedStruct;
    static inline const wchar_t* const CLASS = L"StructProperty";

    static Value get(UStructProperty* prop, uintptr_t addr);
    static void set(UStructProperty* prop, uintptr_t addr, Value value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H */
