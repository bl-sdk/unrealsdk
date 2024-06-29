#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/uenumproperty.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

namespace {

// Have observed uint8, int8, uint32, and int32
// Since negatives are possible, the enum value type is a int64
// This list therefore includes all numeric types *except* uint64, which could go out of range
using valid_underlying_types = std::tuple<UInt8Property,
                                          UInt16Property,
                                          UIntProperty,
                                          UInt64Property,
                                          UByteProperty,
                                          UUInt16Property,
                                          UUInt32Property>;

}  // namespace

UProperty* UEnumProperty::get_underlying_prop(void) const {
    return this->read_field(&UEnumProperty::UnderlyingProp);
}

UEnum* UEnumProperty::get_enum(void) const {
    return this->read_field(&UEnumProperty::Enum);
}

PropTraits<UEnumProperty>::Value PropTraits<UEnumProperty>::get(const UEnumProperty* prop,
                                                                uintptr_t addr,
                                                                const UnrealPointer<void>& parent) {
    PropTraits<UEnumProperty>::Value value{};

    cast<cast_options<>::with_classes<valid_underlying_types>>(
        prop->get_underlying_prop(), [addr, parent, &value]<typename T>(const T* underlying) {
            static_assert(std::is_integral_v<typename PropTraits<T>::Value>);

            using underlying_limits = std::numeric_limits<typename PropTraits<T>::Value>;
            using value_limits = std::numeric_limits<typename PropTraits<UEnumProperty>::Value>;
            static_assert(underlying_limits::min() >= value_limits::min()
                          && underlying_limits::max() <= value_limits::max());

            value = PropTraits<T>::get(underlying, addr, parent);
        });

    return value;
}

void PropTraits<UEnumProperty>::set(const UEnumProperty* prop, uintptr_t addr, const Value& value) {
    cast<cast_options<>::with_classes<valid_underlying_types>>(
        prop->get_underlying_prop(), [addr, value]<typename T>(const T* underlying) {
            static_assert(std::is_integral_v<typename PropTraits<T>::Value>);

            using underlying_limits = std::numeric_limits<typename PropTraits<T>::Value>;
            using value_limits = std::numeric_limits<typename PropTraits<UEnumProperty>::Value>;
            static_assert(underlying_limits::min() >= value_limits::min()
                          && underlying_limits::max() <= value_limits::max());

            if (underlying_limits::min() > value || value > underlying_limits::max()) {
                throw std::invalid_argument("Enum value out of range!");
            }

            PropTraits<T>::set(underlying, addr, static_cast<typename PropTraits<T>::Value>(value));
        });
}

}  // namespace unrealsdk::unreal
