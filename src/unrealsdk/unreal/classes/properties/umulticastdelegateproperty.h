#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UMULTICASTDELEGATEPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UMULTICASTDELEGATEPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_multicast_delegate.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UFunction;

namespace offsets::generic {

template <typename T>
class UMulticastDelegateProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UFunction* Signature;
};

}  // namespace offsets::generic

class UMulticastDelegateProperty : public UProperty {
   public:
    UMulticastDelegateProperty() = delete;
    UMulticastDelegateProperty(const UMulticastDelegateProperty&) = delete;
    UMulticastDelegateProperty(UMulticastDelegateProperty&&) = delete;
    UMulticastDelegateProperty& operator=(const UMulticastDelegateProperty&) = delete;
    UMulticastDelegateProperty& operator=(UMulticastDelegateProperty&&) = delete;
    ~UMulticastDelegateProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UMULTICASTDELEGATEPROPERTY_FIELDS(X) X(UFunction*, Signature)

    UNREALSDK_DEFINE_FIELDS_HEADER(UMulticastDelegateProperty,
                                   UNREALSDK_UMULTICASTDELEGATEPROPERTY_FIELDS);
};

template <>
struct PropTraits<UMulticastDelegateProperty>
    : public AbstractPropTraits<UMulticastDelegateProperty> {
    using Value = WrappedMulticastDelegate;

    static Value get(const UMulticastDelegateProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const UMulticastDelegateProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const UMulticastDelegateProperty* prop, uintptr_t addr);
};

template <>
struct ClassTraits<UMulticastDelegateProperty> {
    static inline const wchar_t* const NAME = L"MulticastDelegateProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UMULTICASTDELEGATEPROPERTY_H */
