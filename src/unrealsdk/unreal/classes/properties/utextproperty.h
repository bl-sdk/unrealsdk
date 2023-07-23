#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UTEXTPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UTEXTPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

// Unlike other properties, which we can define on engine versions which don't support them, text
// properties call into a game-specific hook, which means we can only define them in UE4
#if defined(UE4)

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UTextProperty : public UProperty {
   public:
    UTextProperty() = delete;
    UTextProperty(const UTextProperty&) = delete;
    UTextProperty(UTextProperty&&) = delete;
    UTextProperty& operator=(const UTextProperty&) = delete;
    UTextProperty& operator=(UTextProperty&&) = delete;
    ~UTextProperty() = delete;
};

template <>
struct PropTraits<UTextProperty> : public AbstractPropTraits<UTextProperty> {
    using Value = std::wstring;

    static Value get(const UTextProperty* prop, uintptr_t addr, const UnrealPointer<void>& parent);
    static void set(const UTextProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const UTextProperty* prop, uintptr_t addr);
};

template <>
struct ClassTraits<UTextProperty> {
    static inline const wchar_t* const NAME = L"TextProperty";
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UTEXTPROPERTY_H */
