#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_ATTRIBUTE_PROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_ATTRIBUTE_PROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/prop_traits.h"

namespace unrealsdk::unreal {

class UArrayProperty;
class UProperty;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

template <typename T, typename = std::enable_if_t<std::is_base_of_v<UProperty, T>>>
class AttributeProperty : public T {
   public:
    AttributeProperty() = delete;
    AttributeProperty(const AttributeProperty&) = delete;
    AttributeProperty(AttributeProperty&&) = delete;
    AttributeProperty& operator=(const AttributeProperty&) = delete;
    AttributeProperty& operator=(AttributeProperty&&) = delete;
    ~AttributeProperty() = delete;

    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

   private:
    UArrayProperty* ModifierStackProperty;
    AttributeProperty<T>* OtherAttributeProperty;

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
   public:
    /**
     * @brief Gets the property used for this attribute's modifier stack.
     *
     * @return The modifier stack property.
     */
    [[nodiscard]] UArrayProperty* get_modifier_stack_prop(void) const {
        return this->read_field(&AttributeProperty<T>::ModifierStackProperty);
    }

    /**
     * @brief Gets the other attribute property used for this attribute.
     *
     * @return The other attribute property.
     */
    [[nodiscard]] AttributeProperty<T>* get_other_attribute_property(void) const {
        return this->read_field(&AttributeProperty<T>::OtherAttributeProperty);
    }
};

template <typename T>
struct PropTraits<AttributeProperty<T>> : public PropTraits<T> {};

using UByteAttributeProperty = AttributeProperty<UByteProperty>;
using UFloatAttributeProperty = AttributeProperty<UFloatProperty>;
using UIntAttributeProperty = AttributeProperty<UIntProperty>;

template <>
inline const wchar_t* const ClassTraits<UByteAttributeProperty>::NAME = L"ByteAttributeProperty";
template <>
inline const wchar_t* const ClassTraits<UFloatAttributeProperty>::NAME = L"FloatAttributeProperty";
template <>
inline const wchar_t* const ClassTraits<UIntAttributeProperty>::NAME = L"IntAttributeProperty";

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_ATTRIBUTE_PROPERTY_H */
