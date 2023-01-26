#ifndef UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/prop_traits.h"

namespace unrealsdk::unreal {

class UClass;
class UObject;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UObjectProperty : public UProperty {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* PropertyClass;
};

template <>
struct PropTraits<UObjectProperty> {
    using Value = UObject*;
    static inline const wchar_t* const CLASS = L"ObjectProperty";

    static Value get(UObjectProperty* prop, uintptr_t addr);
    static void set(UObjectProperty* prop, uintptr_t addr, Value value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H */
