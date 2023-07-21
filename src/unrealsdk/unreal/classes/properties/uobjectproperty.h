#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

class UClass;
class UObject;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UObjectProperty : public UProperty {
   public:
    UObjectProperty() = delete;
    UObjectProperty(const UObjectProperty&) = delete;
    UObjectProperty(UObjectProperty&&) = delete;
    UObjectProperty& operator=(const UObjectProperty&) = delete;
    UObjectProperty& operator=(UObjectProperty&&) = delete;
    ~UObjectProperty() = delete;

    /**
     * @brief Get the class of this property, which values must be an instance of.
     *
     * @return This property's class.
     */
    [[nodiscard]] UClass* get_property_class(void) const;

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* PropertyClass;
};

template <>
struct PropTraits<UObjectProperty> : public AbstractPropTraits<UObjectProperty> {
    using Value = UObject*;

    static Value get(const UObjectProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const UObjectProperty* prop, uintptr_t addr, const Value& value);
};

template <>
struct ClassTraits<UObjectProperty> {
    static inline const wchar_t* const NAME = L"ObjectProperty";
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H */
