#ifndef UNREAL_CLASSES_PROPERTIES_UINTERFACEPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_UINTERFACEPROPERTY_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/prop_traits.h"

namespace unrealsdk::unreal {

class UClass;
class UObject;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UInterfaceProperty : public UProperty {
   public:
    UInterfaceProperty() = delete;
    UInterfaceProperty(const UInterfaceProperty&) = delete;
    UInterfaceProperty(UInterfaceProperty&&) = delete;
    UInterfaceProperty& operator=(const UInterfaceProperty&) = delete;
    UInterfaceProperty& operator=(UInterfaceProperty&&) = delete;
    ~UInterfaceProperty() = delete;

   private:
    friend PropTraits<UInterfaceProperty>;

    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* InterfaceClass;
};

template <>
struct PropTraits<UInterfaceProperty> : public AbstractPropTraits<UInterfaceProperty> {
    using Value = UObject*;
    static inline const wchar_t* const CLASS = L"InterfaceProperty";

    static Value get(const UInterfaceProperty* prop,
                     uintptr_t addr,
                     const std::shared_ptr<void>& parent);
    static void set(const UInterfaceProperty* prop, uintptr_t addr, const Value& value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_UINTERFACEPROPERTY_H */
