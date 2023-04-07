#ifndef UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H

#include "pch.h"

#include "unreal/classes/properties/uobjectproperty.h"
#include "unreal/prop_traits.h"

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

   private:
    friend PropTraits<UClassProperty>;

    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* MetaClass;
};

template <>
struct PropTraits<UClassProperty> : public AbstractPropTraits<UClassProperty> {
    using Value = UClass*;
    static inline const wchar_t* const CLASS = L"ClassProperty";

    static Value get(const UClassProperty* prop,
                     uintptr_t addr,
                     const std::shared_ptr<void>& parent);
    static void set(const UClassProperty* prop, uintptr_t addr, const Value& value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_UCLASSPROPERTY_H */
