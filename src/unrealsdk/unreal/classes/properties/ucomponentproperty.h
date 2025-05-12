#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCOMPONENTPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCOMPONENTPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/prop_traits.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UComponentProperty : public UObjectProperty {
   public:
    UComponentProperty() = delete;
    UComponentProperty(const UComponentProperty&) = delete;
    UComponentProperty(UComponentProperty&&) = delete;
    UComponentProperty& operator=(const UComponentProperty&) = delete;
    UComponentProperty& operator=(UComponentProperty&&) = delete;
    ~UComponentProperty() = delete;
};

template <>
struct PropTraits<UComponentProperty> : public PropTraits<UObjectProperty> {};

template <>
struct ClassTraits<UComponentProperty> {
    static inline const wchar_t* const NAME = L"ComponentProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCOMPONENTPROPERTY_H */
