#include "pch.h"

#include "unreal/cast_prop.h"
#include "unreal/classes/properties/uarrayproperty.h"
#include "unreal/prop_traits.h"
#include "unreal/structs/tarray.h"
#include "unreal/structs/tarray_funcs.h"
#include "unreal/wrappers/wrapped_array.h"

namespace unrealsdk::unreal {

PropTraits<UArrayProperty>::Value PropTraits<UArrayProperty>::get(
    const UArrayProperty* prop,
    uintptr_t addr,
    const std::shared_ptr<void>& parent) {
    auto inner = prop->read_field(&UArrayProperty::Inner);
    if (prop->ArrayDim > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }

    return {inner, reinterpret_cast<TArray<void>*>(addr), parent};
}

void PropTraits<UArrayProperty>::set(const UArrayProperty* prop,
                                     uintptr_t addr,
                                     const Value& value) {
    auto inner = prop->read_field(&UArrayProperty::Inner);
    if (prop->ArrayDim > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }
    if (value.type != inner) {
        throw std::runtime_error("Array does not contain fields of type "
                                 + (std::string)inner->Name);
    }

    cast_prop(inner, [prop, addr, &value]<typename T>(const T* inner) {
        auto arr = reinterpret_cast<TArray<void>*>(addr);
        auto data_addr = reinterpret_cast<uintptr_t>(arr->data);

        auto new_size = value.size();
        auto current_size = arr->size();

        // If the new size is smaller, destroy anything dropping off the end
        for (size_t i = new_size; i < current_size; i++) {
            destroy_property<T>(inner, i, data_addr);
        }

        arr->resize(new_size, prop->ElementSize);

        for (size_t i = 0; i < new_size; i++) {
            set_property<T>(inner, i, data_addr, value.get_at<T>(i));
        }
    });
}

void PropTraits<UArrayProperty>::destroy(const UArrayProperty* prop, uintptr_t addr) {
    auto inner = prop->read_field(&UArrayProperty::Inner);
    if (prop->ArrayDim > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }

    auto arr = reinterpret_cast<TArray<void>*>(addr);

    cast_prop(inner, [arr]<typename T>(const T* inner) {
        for (size_t i = 0; i < arr->size(); i++) {
            destroy_property<T>(inner, i, reinterpret_cast<uintptr_t>(arr->data));
        }
    });

    u_free(arr->data);
    arr->data = nullptr;
}

}  // namespace unrealsdk::unreal
