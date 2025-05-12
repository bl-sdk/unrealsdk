#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBYTEPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBYTEPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/prop_traits.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UEnum;

namespace offsets::generic {

template <typename T>
class UByteProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UEnum* Enum;
};

}  // namespace offsets::generic

class UByteProperty : public CopyableProperty<uint8_t> {
   public:
    UByteProperty() = delete;
    UByteProperty(const UByteProperty&) = delete;
    UByteProperty(UByteProperty&&) = delete;
    UByteProperty& operator=(const UByteProperty&) = delete;
    UByteProperty& operator=(UByteProperty&&) = delete;
    ~UByteProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UBYTEPROPERTY_FIELDS(X) X(UEnum*, Enum)

    UNREALSDK_DEFINE_FIELDS_HEADER(UByteProperty, UNREALSDK_UBYTEPROPERTY_FIELDS);

   private:
    // NOLINTBEGIN(readability-identifier-naming)

    UEnum* Enum_member;

    // NOLINTEND(readability-identifier-naming)
};

template <>
struct PropTraits<UByteProperty> : public PropTraits<CopyableProperty<uint8_t>> {};

template <>
struct ClassTraits<UByteProperty> {
    static inline const wchar_t* const NAME = L"ByteProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBYTEPROPERTY_H */
