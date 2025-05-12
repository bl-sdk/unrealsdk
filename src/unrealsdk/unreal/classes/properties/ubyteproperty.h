#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBYTEPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBYTEPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/prop_traits.h"

namespace unrealsdk::unreal {

class UEnum;

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UByteProperty : public CopyableProperty<uint8_t> {
   public:
    UByteProperty() = delete;
    UByteProperty(const UByteProperty&) = delete;
    UByteProperty(UByteProperty&&) = delete;
    UByteProperty& operator=(const UByteProperty&) = delete;
    UByteProperty& operator=(UByteProperty&&) = delete;
    ~UByteProperty() = delete;

   private:
    // NOLINTBEGIN(readability-identifier-naming)

    UEnum* Enum;

    // NOLINTEND(readability-identifier-naming)

   public:
    /**
     * @brief Get the enum associated with this property, if any.
     *
     * @return The enum.
     */
    [[nodiscard]] UEnum* get_enum(void) const;
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
