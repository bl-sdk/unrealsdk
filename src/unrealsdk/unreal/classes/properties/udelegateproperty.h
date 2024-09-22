#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UDELEGATEPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UDELEGATEPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UFunction;

class UDelegateProperty : public UProperty {
   public:
    UDelegateProperty() = delete;
    UDelegateProperty(const UDelegateProperty&) = delete;
    UDelegateProperty(UDelegateProperty&&) = delete;
    UDelegateProperty& operator=(const UDelegateProperty&) = delete;
    UDelegateProperty& operator=(UDelegateProperty&&) = delete;
    ~UDelegateProperty() = delete;

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UFunction* Signature;

   public:
    /**
     * @brief Get the function holding this delegate's signature.
     *
     * @return The signature function.
     */
    [[nodiscard]] UFunction* get_signature(void) const;
};

template <>
struct PropTraits<UDelegateProperty> : public AbstractPropTraits<UDelegateProperty> {
    using Value = std::optional<BoundFunction>;

    static Value get(const UDelegateProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const UDelegateProperty* prop, uintptr_t addr, const Value& value);
};

template <>
struct ClassTraits<UDelegateProperty> {
    static inline const wchar_t* const NAME = L"DelegateProperty";
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UDELEGATEPROPERTY_H */
