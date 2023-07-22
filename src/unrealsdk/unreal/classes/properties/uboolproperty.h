#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBOOLPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBOOLPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

class UBoolProperty : public UProperty {
   public:
    UBoolProperty() = delete;
    UBoolProperty(const UBoolProperty&) = delete;
    UBoolProperty(UBoolProperty&&) = delete;
    UBoolProperty& operator=(const UBoolProperty&) = delete;
    UBoolProperty& operator=(UBoolProperty&&) = delete;
    ~UBoolProperty() = delete;

   private:
    // NOLINTBEGIN(readability-identifier-naming)

#ifdef UE4
    uint8_t FieldSize;
    uint8_t ByteOffset;
    uint8_t ByteMask;
    uint8_t FieldMask;
#else
    uint32_t FieldMask;
#endif

    // NOLINTEND(readability-identifier-naming)
   public:
    /**
     * @brief Get the bool field mask of this property.
     *
     * @return The field mask.
     */
    [[nodiscard]] decltype(FieldMask) get_field_mask(void) const;
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

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UBOOLPROPERTY_H */
