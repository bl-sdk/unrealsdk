#include "pch.h"

#include "unreal/classes/properties/uarrayproperty.h"
#include "unreal/structs/tarray.h"
#include "unreal/structs/tarray_funcs.h"
#include "unreal/wrappers/wrapped_array.h"


namespace unrealsdk::unreal {

PropTraits<UArrayProperty>::Value PropTraits<UArrayProperty>::get(const UArrayProperty* prop,
                                                                    uintptr_t addr) {
    auto inner = prop->read_field(&UArrayProperty::Inner);
    return {inner, reinterpret_cast<TArray<void>*>(addr)};
}

void PropTraits<UArrayProperty>::set(const UArrayProperty* prop, uintptr_t addr, const Value& value) {
    auto inner = prop->read_field(&UArrayProperty::Inner);
    if (value.type != inner) {
        throw std::runtime_error("Array does not contain fields of type " + (std::string)inner->Name);
    }

    auto arr = reinterpret_cast<TArray<void>*>(addr);

    auto new_size = value.size();
    if (new_size >= arr->capacity()) {
        arr->reserve(new_size, prop->ElementSize);
    }

    memcpy(arr->data, value.base->data, new_size * prop->ElementSize);

    // Rely on `TArray::reserve` to detect overruns
    arr->count = static_cast<decltype(arr->count)>(new_size);
}

}  // namespace unrealsdk::unreal
