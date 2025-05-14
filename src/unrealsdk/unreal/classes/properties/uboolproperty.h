#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBOOLPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBOOLPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

class UBoolProperty : public UProperty {
   public:
    UBoolProperty() = delete;
    UBoolProperty(const UBoolProperty&) = delete;
    UBoolProperty(UBoolProperty&&) = delete;
    UBoolProperty& operator=(const UBoolProperty&) = delete;
    UBoolProperty& operator=(UBoolProperty&&) = delete;
    ~UBoolProperty() = delete;

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    using field_mask_type = uint8_t;
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    using field_mask_type = uint32_t;
#else
#error Unknown SDK flavour
#endif

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UBOOLPROPERTY_FIELDS(X) X(field_mask_type, FieldMask)

    UNREALSDK_DEFINE_FIELDS_HEADER(UBoolProperty, UNREALSDK_UBOOLPROPERTY_FIELDS);
};

template <>
struct PropTraits<UBoolProperty> : public AbstractPropTraits<UBoolProperty> {
    using Value = bool;

    static Value get(const UBoolProperty* prop, uintptr_t addr, const UnrealPointer<void>& parent);
    static void set(const UBoolProperty* prop, uintptr_t addr, const Value& value);
};

template <>
struct ClassTraits<UBoolProperty> {
    static inline const wchar_t* const NAME = L"BoolProperty";
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBOOLPROPERTY_H */
