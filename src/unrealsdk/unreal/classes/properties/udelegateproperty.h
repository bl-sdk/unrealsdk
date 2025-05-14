#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UDELEGATEPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UDELEGATEPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UFunction;

namespace offsets::generic {

template <typename T>
class UDelegateProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UFunction* Signature;
};

}  // namespace offsets::generic

class UDelegateProperty : public UProperty {
   public:
    UDelegateProperty() = delete;
    UDelegateProperty(const UDelegateProperty&) = delete;
    UDelegateProperty(UDelegateProperty&&) = delete;
    UDelegateProperty& operator=(const UDelegateProperty&) = delete;
    UDelegateProperty& operator=(UDelegateProperty&&) = delete;
    ~UDelegateProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UDELEGATEPROPERTY_FIELDS(X) X(UFunction*, Signature)

    UNREALSDK_DEFINE_FIELDS_HEADER(UDelegateProperty, UNREALSDK_UDELEGATEPROPERTY_FIELDS);
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

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UDELEGATEPROPERTY_H */
