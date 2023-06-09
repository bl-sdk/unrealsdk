#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_COPYABLE_PROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_COPYABLE_PROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"

/*
This file describes all properties which can be read/written by a simple copy, and whose classes
 don't need to add any extra fields to `UProperty` (or at least not any we care about).
*/

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

template <typename T>
class CopyableProperty : public UProperty {
   public:
    CopyableProperty<T>() = delete;
    CopyableProperty<T>(const CopyableProperty<T>&) = delete;
    CopyableProperty<T>(CopyableProperty<T>&&) = delete;
    CopyableProperty<T>& operator=(const CopyableProperty<T>&) = delete;
    CopyableProperty<T>& operator=(CopyableProperty<T>&&) = delete;
    ~CopyableProperty<T>() = delete;
};

template <typename T>
struct PropTraits<CopyableProperty<T>> : public AbstractPropTraits<CopyableProperty<T>> {
    using Value = T;
    static const wchar_t* const CLASS;

    static Value get(const CopyableProperty<T>* /*prop*/,
                     uintptr_t addr,
                     const std::shared_ptr<void>& /*parent*/) {
        return *reinterpret_cast<Value*>(addr);
    }
    static void set(const CopyableProperty<T>* /*prop*/, uintptr_t addr, const Value& value) {
        *reinterpret_cast<Value*>(addr) = value;
    }
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
inline const wchar_t* const PropTraits<UNameProperty>::CLASS = L"NameProperty";

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_COPYABLE_PROPERTY_H */
