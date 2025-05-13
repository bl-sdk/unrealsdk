#ifndef PYUNREALSDK_LIBS_UNREALSDK_SRC_UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H
#define PYUNREALSDK_LIBS_UNREALSDK_SRC_UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UClass;

namespace offsets::generic {

template <typename T>
class UClassProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* MetaClass;
};

}  // namespace offsets::generic

class UClassProperty : public UObjectProperty {
   public:
    UClassProperty() = delete;
    UClassProperty(const UClassProperty&) = delete;
    UClassProperty(UClassProperty&&) = delete;
    UClassProperty& operator=(const UClassProperty&) = delete;
    UClassProperty& operator=(UClassProperty&&) = delete;
    ~UClassProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UCLASSPROPERTY_FIELDS(X) X(UClass*, MetaClass)

    UNREALSDK_DEFINE_FIELDS_HEADER(UClassProperty, UNREALSDK_UCLASSPROPERTY_FIELDS);

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* MetaClass_member;
};

template <>
struct PropTraits<UClassProperty> : public AbstractPropTraits<UClassProperty> {
    using Value = UClass*;

    static Value get(const UClassProperty* prop, uintptr_t addr, const UnrealPointer<void>& parent);
    static void set(const UClassProperty* prop, uintptr_t addr, const Value& value);
};

template <>
struct ClassTraits<UClassProperty> {
    static inline const wchar_t* const NAME = L"ClassProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* PYUNREALSDK_LIBS_UNREALSDK_SRC_UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H */
