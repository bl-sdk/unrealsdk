#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UArrayProperty, UNREALSDK_UARRAYPROPERTY_FIELDS);

PropTraits<UArrayProperty>::Value PropTraits<UArrayProperty>::get(
    const UArrayProperty* prop,
    uintptr_t addr,
    const UnrealPointer<void>& parent) {
    auto inner = prop->Inner();
    if (prop->ArrayDim() > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }

    return {inner, reinterpret_cast<TArray<void>*>(addr), parent};
}

void PropTraits<UArrayProperty>::set(const UArrayProperty* prop,
                                     uintptr_t addr,
                                     const Value& value) {
    auto inner = prop->Inner();
    if (prop->ArrayDim() > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }
    if (value.type != inner) {
        throw std::runtime_error(
            utils::narrow(std::format(L"Array fields have incompatible type, expected {}, got {}",
                                      inner->get_path_name(), value.type->get_path_name())));
    }

    auto arr = reinterpret_cast<TArray<void>*>(addr);
    if (arr->data != nullptr && arr->data == value.base.get()->data) {
        LOG(DEV_WARNING, L"Refusing to set array property {} to itself, at address {:p}",
            prop->get_path_name(), reinterpret_cast<void*>(addr));
        return;
    }

    cast(inner, [&arr, &value]<typename T>(const T* inner) {
        auto new_size = value.size();
        arr->resize(new_size, inner->ElementSize());

        for (size_t i = 0; i < new_size; i++) {
            set_property<T>(inner, 0,
                            reinterpret_cast<uintptr_t>(arr->data) + (inner->ElementSize() * i),
                            value.get_at<T>(i));
        }
    });
}

void PropTraits<UArrayProperty>::destroy(const UArrayProperty* prop, uintptr_t addr) {
    auto inner = prop->Inner();
    if (prop->ArrayDim() > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }

    auto arr = reinterpret_cast<TArray<void>*>(addr);

    cast(inner, [arr]<typename T>(const T* inner) {
        for (size_t i = 0; i < arr->size(); i++) {
            destroy_property<T>(
                inner, 0, reinterpret_cast<uintptr_t>(arr->data) + (inner->ElementSize() * i));
        }
    });

    arr->free();
}

}  // namespace unrealsdk::unreal
