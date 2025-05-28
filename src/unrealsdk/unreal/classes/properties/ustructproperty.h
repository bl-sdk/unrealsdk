#ifndef PYUNREALSDK_LIBS_UNREALSDK_SRC_UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H
#define PYUNREALSDK_LIBS_UNREALSDK_SRC_UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UScriptStruct;

namespace offsets::generic {

template <typename T>
class UStructProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    unreal::UScriptStruct* Struct;
};

}  // namespace offsets::generic

class UStructProperty : public UProperty {
   public:
    UStructProperty() = delete;
    UStructProperty(const UStructProperty&) = delete;
    UStructProperty(UStructProperty&&) = delete;
    UStructProperty& operator=(const UStructProperty&) = delete;
    UStructProperty& operator=(UStructProperty&&) = delete;
    ~UStructProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_USTRUCTPROPERTY_FIELDS(X) X(UScriptStruct*, Struct)

    UNREALSDK_DEFINE_FIELDS_HEADER(UStructProperty, UNREALSDK_USTRUCTPROPERTY_FIELDS);
};

template <>
struct PropTraits<UStructProperty> : public AbstractPropTraits<UStructProperty> {
    using Value = WrappedStruct;

    static Value get(const UStructProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const UStructProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const UStructProperty* prop, uintptr_t addr);
};

template <>
struct ClassTraits<UStructProperty> {
    static inline const wchar_t* const NAME = L"StructProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* PYUNREALSDK_LIBS_UNREALSDK_SRC_UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H */
