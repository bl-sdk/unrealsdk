#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

class UClass;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UClassProperty : public UObjectProperty {
   public:
    UClassProperty() = delete;
    UClassProperty(const UClassProperty&) = delete;
    UClassProperty(UClassProperty&&) = delete;
    UClassProperty& operator=(const UClassProperty&) = delete;
    UClassProperty& operator=(UClassProperty&&) = delete;
    ~UClassProperty() = delete;

    /**
     * @brief Get the meta class of this property, which values must be a subclass of.
     *
     * @return This property's meta class.
     */
    [[nodiscard]] UClass* get_meta_class(void) const;

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* MetaClass;
};

template <>
struct PropTraits<UClassProperty> : public AbstractPropTraits<UClassProperty> {
    using Value = UClass*;
    static inline const wchar_t* const CLASS = L"ClassProperty";

    static Value get(const UClassProperty* prop, uintptr_t addr, const UnrealPointer<void>& parent);
    static void set(const UClassProperty* prop, uintptr_t addr, const Value& value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H */
