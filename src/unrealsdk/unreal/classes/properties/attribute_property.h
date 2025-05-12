#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_ATTRIBUTE_PROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_ATTRIBUTE_PROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/prop_traits.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UArrayProperty;
class UProperty;

namespace offsets::generic {

template <typename T>
class GenericAttributeProperty : public T {
   public:
    // NOLINTBEGIN(readability-identifier-naming)

    UArrayProperty* ModifierStackProperty;
    GenericAttributeProperty<T>* OtherAttributeProperty;

    // NOLINTEND(readability-identifier-naming)
};

}  // namespace offsets::generic

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_ATTRIBUTE_PROPERTY(class_name, base_class, fields) \
    class class_name : public base_class {                        \
       public:                                                    \
        class_name() = delete;                                    \
        class_name(const class_name&) = delete;                   \
        class_name(class_name&&) = delete;                        \
        class_name& operator=(const class_name&) = delete;        \
        class_name& operator=(class_name&&) = delete;             \
        ~class_name() = delete;                                   \
        /* NOLINTNEXTLINE(readability-identifier-naming) */       \
        UNREALSDK_DEFINE_FIELDS_HEADER(class_name, fields);       \
    };                                                            \
    template <>                                                   \
    struct PropTraits<class_name> : public PropTraits<base_class> {};

// These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UBYTEATTRIBUTEPROPERTY_FIELDS(X) \
    X(UArrayProperty*, ModifierStackProperty)      \
    X(UByteAttributeProperty*, OtherAttributeProperty)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UFLOATATTRIBUTEPROPERTY_FIELDS(X) \
    X(UArrayProperty*, ModifierStackProperty)       \
    X(UFloatAttributeProperty*, OtherAttributeProperty)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UINTATTRIBUTEPROPERTY_FIELDS(X) \
    X(UArrayProperty*, ModifierStackProperty)     \
    X(UIntAttributeProperty*, OtherAttributeProperty)

DEFINE_ATTRIBUTE_PROPERTY(UByteAttributeProperty,
                          UByteProperty,
                          UNREALSDK_UBYTEATTRIBUTEPROPERTY_FIELDS);
DEFINE_ATTRIBUTE_PROPERTY(UFloatAttributeProperty,
                          UFloatProperty,
                          UNREALSDK_UFLOATATTRIBUTEPROPERTY_FIELDS);
DEFINE_ATTRIBUTE_PROPERTY(UIntAttributeProperty,
                          UIntProperty,
                          UNREALSDK_UINTATTRIBUTEPROPERTY_FIELDS);

template <>
inline const wchar_t* const ClassTraits<UByteAttributeProperty>::NAME = L"ByteAttributeProperty";
template <>
inline const wchar_t* const ClassTraits<UFloatAttributeProperty>::NAME = L"FloatAttributeProperty";
template <>
inline const wchar_t* const ClassTraits<UIntAttributeProperty>::NAME = L"IntAttributeProperty";

#undef DEFINE_ATTRIBUTE_PROPERTY

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_ATTRIBUTE_PROPERTY_H */
