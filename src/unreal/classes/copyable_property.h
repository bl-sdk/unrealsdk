#ifndef UNREAL_CLASSES_PROPERTIES_NUMERIC_H
#define UNREAL_CLASSES_PROPERTIES_NUMERIC_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/prop_traits.h"

/*
This file describes all properties which can be read/written by a simple copy, and whoose classes
 don't need to add any extra fields to `UProperty`.
This isn't always the same as what you'd intuitively call a value vs reference type, for example:

`UObjectProperty` is copyable, even though it's a pointer, since we can copy the pointer (by value)
 to have our own reference.
`UStructProperty` is not copyable, even though it's a value type, since we need to wrap it into a
 `WrappedStruct` to be able to access it's inner fields.
*/

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

template <typename T>
class CopyableProperty : public UProperty {};

template <typename T>
struct PropTraits<CopyableProperty<T>> {
    using Value = T;
    static const wchar_t* const CLASS;

    static Value get(uintptr_t addr) { return *reinterpret_cast<Value*>(addr); }
    static void set(uintptr_t addr, Value value) { *reinterpret_cast<Value*>(addr) = value; }
};

using UInt8Property = CopyableProperty<int8_t>;
using UInt16Property = CopyableProperty<int16_t>;
using UIntProperty = CopyableProperty<int32_t>;
using UInt64Property = CopyableProperty<int64_t>;

using UByteProperty = CopyableProperty<uint8_t>;
using UUInt16Property = CopyableProperty<uint16_t>;
using UUInt32Property = CopyableProperty<uint32_t>;
using UUInt64Property = CopyableProperty<uint64_t>;

using UFloatProperty = CopyableProperty<float32_t>;
using UDoubleProperty = CopyableProperty<float64_t>;

using UObjectProperty = CopyableProperty<UObject*>;
using UClassProperty = CopyableProperty<UClass*>;
using UNameProperty = CopyableProperty<FName>;

template <>
inline const wchar_t* const PropTraits<UInt8Property>::CLASS = L"Int8Property";
template <>
inline const wchar_t* const PropTraits<UInt16Property>::CLASS = L"Int16Property";
template <>
inline const wchar_t* const PropTraits<UIntProperty>::CLASS = L"IntProperty";
template <>
inline const wchar_t* const PropTraits<UInt64Property>::CLASS = L"Int64Property";

template <>
inline const wchar_t* const PropTraits<UByteProperty>::CLASS = L"ByteProperty";
template <>
inline const wchar_t* const PropTraits<UUInt16Property>::CLASS = L"UInt16Property";
template <>
inline const wchar_t* const PropTraits<UUInt32Property>::CLASS = L"UInt32Property";
template <>
inline const wchar_t* const PropTraits<UUInt64Property>::CLASS = L"UInt64Property";

template <>
inline const wchar_t* const PropTraits<UFloatProperty>::CLASS = L"FloatProperty";
template <>
inline const wchar_t* const PropTraits<UDoubleProperty>::CLASS = L"DoubleProperty";

template <>
inline const wchar_t* const PropTraits<UObjectProperty>::CLASS = L"ObjectProperty";
template <>
inline const wchar_t* const PropTraits<UClassProperty>::CLASS = L"ClassProperty";
template <>
inline const wchar_t* const PropTraits<UNameProperty>::CLASS = L"NameProperty";

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_NUMERIC_H */
