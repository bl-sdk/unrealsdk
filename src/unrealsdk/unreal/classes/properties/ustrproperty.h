#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
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
struct PropTraits<UStrProperty> : public AbstractPropTraits<UStrProperty> {
    using Value = std::wstring;

    static Value get(const UStrProperty* prop, uintptr_t addr, const UnrealPointer<void>& parent);
    static void set(const UStrProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const UStrProperty* prop, uintptr_t addr);
};

template <>
struct ClassTraits<UStrProperty> {
    static inline const wchar_t* const NAME = L"StrProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRPROPERTY_H */
