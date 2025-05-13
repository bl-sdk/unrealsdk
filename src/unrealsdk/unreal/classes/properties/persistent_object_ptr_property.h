#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_PERSISTENT_OBJECT_PTR_PROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_PERSISTENT_OBJECT_PTR_PROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

struct FLazyObjectPath;
struct FSoftObjectPath;
class UObject;

namespace offsets::generic {

template <typename T>
class USoftClassProperty : public T {
   public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* MetaClass;
};

}  // namespace offsets::generic

class ULazyObjectProperty : public UObjectProperty {
   public:
    ULazyObjectProperty() = delete;
    ULazyObjectProperty(const ULazyObjectProperty&) = delete;
    ULazyObjectProperty(ULazyObjectProperty&&) = delete;
    ULazyObjectProperty& operator=(const ULazyObjectProperty&) = delete;
    ULazyObjectProperty& operator=(ULazyObjectProperty&&) = delete;
    ~ULazyObjectProperty() = delete;
};

class USoftObjectProperty : public UObjectProperty {
   public:
    USoftObjectProperty() = delete;
    USoftObjectProperty(const USoftObjectProperty&) = delete;
    USoftObjectProperty(USoftObjectProperty&&) = delete;
    USoftObjectProperty& operator=(const USoftObjectProperty&) = delete;
    USoftObjectProperty& operator=(USoftObjectProperty&&) = delete;
    ~USoftObjectProperty() = delete;
};

// Not entirely sure if this should inherit soft object property or class property, but soft object
// object simplifies a lot of our code
class USoftClassProperty : public USoftObjectProperty {
   public:
    USoftClassProperty() = delete;
    USoftClassProperty(const USoftClassProperty&) = delete;
    USoftClassProperty(USoftClassProperty&&) = delete;
    USoftClassProperty& operator=(const USoftClassProperty&) = delete;
    USoftClassProperty& operator=(USoftClassProperty&&) = delete;
    ~USoftClassProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_USOFTCLASSPROPERTY_FIELDS(X) X(UClass*, MetaClass)

    UNREALSDK_DEFINE_FIELDS_HEADER(USoftClassProperty, UNREALSDK_USOFTCLASSPROPERTY_FIELDS);

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* MetaClass_member;
};

template <>
struct PropTraits<ULazyObjectProperty> : public AbstractPropTraits<ULazyObjectProperty> {
    using Value = UObject*;

    static Value get(const ULazyObjectProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const ULazyObjectProperty* prop, uintptr_t addr, const Value& value);

    static const FLazyObjectPath* get_identifier(const ULazyObjectProperty* prop,
                                                 uintptr_t addr,
                                                 const UnrealPointer<void>& parent);
};

template <>
struct PropTraits<USoftObjectProperty> : public AbstractPropTraits<USoftObjectProperty> {
    using Value = UObject*;

    static Value get(const USoftObjectProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const USoftObjectProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const USoftObjectProperty* prop, uintptr_t addr);

    static const FSoftObjectPath* get_identifier(const USoftObjectProperty* prop,
                                                 uintptr_t addr,
                                                 const UnrealPointer<void>& parent);
};

template <>
struct PropTraits<USoftClassProperty> : public AbstractPropTraits<USoftClassProperty> {
    using Value = UClass*;

    static Value get(const USoftClassProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const USoftClassProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const USoftClassProperty* prop, uintptr_t addr);

    static const FSoftObjectPath* get_identifier(const USoftClassProperty* prop,
                                                 uintptr_t addr,
                                                 const UnrealPointer<void>& parent);
};

template <>
struct ClassTraits<ULazyObjectProperty> {
    static inline const wchar_t* const NAME = L"LazyObjectProperty";
};
template <>
struct ClassTraits<USoftObjectProperty> {
    static inline const wchar_t* const NAME = L"SoftObjectProperty";
};
template <>
struct ClassTraits<USoftClassProperty> {
    static inline const wchar_t* const NAME = L"SoftClassProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_PERSISTENT_OBJECT_PTR_PROPERTY_H */
