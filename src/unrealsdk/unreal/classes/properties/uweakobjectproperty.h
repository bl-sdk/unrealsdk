#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UWEAKOBJECTPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UWEAKOBJECTPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

class UObject;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UWeakObjectProperty : public UObjectProperty {
   public:
    UWeakObjectProperty() = delete;
    UWeakObjectProperty(const UWeakObjectProperty&) = delete;
    UWeakObjectProperty(UWeakObjectProperty&&) = delete;
    UWeakObjectProperty& operator=(const UWeakObjectProperty&) = delete;
    UWeakObjectProperty& operator=(UWeakObjectProperty&&) = delete;
    ~UWeakObjectProperty() = delete;
};

template <>
struct PropTraits<UWeakObjectProperty> : public AbstractPropTraits<UWeakObjectProperty> {
    using Value = UObject*;

    static Value get(const UWeakObjectProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const UWeakObjectProperty* prop, uintptr_t addr, const Value& value);
};

template <>
struct ClassTraits<UWeakObjectProperty> {
    static inline const wchar_t* const NAME = L"WeakObjectProperty";
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UWEAKOBJECTPROPERTY_H */
