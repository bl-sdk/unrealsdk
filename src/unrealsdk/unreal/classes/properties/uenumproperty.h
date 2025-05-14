#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UENUMPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UENUMPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UEnum;

namespace offsets::generic {

template <typename T>
class UEnumProperty : public T {
   public:
    // NOLINTBEGIN(readability-identifier-naming)

    UProperty* UnderlyingProp;
    UEnum* Enum;

    // NOLINTEND(readability-identifier-naming)
};

}  // namespace offsets::generic

class UEnumProperty : public UProperty {
   public:
    UEnumProperty() = delete;
    UEnumProperty(const UEnumProperty&) = delete;
    UEnumProperty(UEnumProperty&&) = delete;
    UEnumProperty& operator=(const UEnumProperty&) = delete;
    UEnumProperty& operator=(UEnumProperty&&) = delete;
    ~UEnumProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UENUMPROPERTY_FIELDS(X) \
    X(UProperty*, UnderlyingProp)         \
    X(UEnum*, Enum)

    UNREALSDK_DEFINE_FIELDS_HEADER(UEnumProperty, UNREALSDK_UENUMPROPERTY_FIELDS);

   private:
    // NOLINTBEGIN(readability-identifier-naming)

    UProperty* UnderlyingProp_member;
    UEnum* Enum_member;

    // NOLINTEND(readability-identifier-naming)
};

template <>
struct PropTraits<UEnumProperty> : public AbstractPropTraits<UEnumProperty> {
    using Value = int64_t;

    static Value get(const UEnumProperty* prop, uintptr_t addr, const UnrealPointer<void>& parent);
    static void set(const UEnumProperty* prop, uintptr_t addr, const Value& value);
};

template <>
struct ClassTraits<UEnumProperty> {
    static inline const wchar_t* const NAME = L"EnumProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UENUMPROPERTY_H */
