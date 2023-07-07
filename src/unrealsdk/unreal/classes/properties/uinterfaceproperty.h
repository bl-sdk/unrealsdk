#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UINTERFACEPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UINTERFACEPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

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

    /**
     * @brief Get the interface class of this property, which values must be an implementation of.
     *
     * @return This property's interface class.
     */
    [[nodiscard]] UClass* get_interface_class(void) const;

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* InterfaceClass;
};

template <>
struct PropTraits<UInterfaceProperty> : public AbstractPropTraits<UInterfaceProperty> {
    using Value = UObject*;
    static inline const wchar_t* const CLASS = L"InterfaceProperty";

    static Value get(const UInterfaceProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const UInterfaceProperty* prop, uintptr_t addr, const Value& value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UINTERFACEPROPERTY_H */
