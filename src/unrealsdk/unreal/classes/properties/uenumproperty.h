#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UENUMPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UENUMPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

class UEnum;

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UEnumProperty : public UProperty {
   public:
    UEnumProperty() = delete;
    UEnumProperty(const UEnumProperty&) = delete;
    UEnumProperty(UEnumProperty&&) = delete;
    UEnumProperty& operator=(const UEnumProperty&) = delete;
    UEnumProperty& operator=(UEnumProperty&&) = delete;
    ~UEnumProperty() = delete;

   private:
    // NOLINTBEGIN(readability-identifier-naming)

    UProperty* UnderlyingProp;
    UEnum* Enum;

    // NOLINTEND(readability-identifier-naming)
   public:
    /**
     * @brief Get the underlying numeric property of this enum property.
     *
     * @return The underlying property.
     */
    [[nodiscard]] UProperty* get_underlying_prop(void) const;

    /**
     * @brief Get the enum associated with this property.
     *
     * @return The enum.
     */
    [[nodiscard]] UEnum* get_enum(void) const;
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
