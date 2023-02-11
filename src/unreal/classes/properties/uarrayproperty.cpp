#include "pch.h"

#include "unreal/classes/properties/uarrayproperty.h"
#include "unreal/structs/tarray.h"
#include "unreal/wrappers/wrapped_array.h"


namespace unrealsdk::unreal {

PropTraits<UArrayProperty>::Value PropTraits<UArrayProperty>::get(const UArrayProperty* prop,
                                                                    uintptr_t addr) {
    auto inner = prop->read_field(&UArrayProperty::Inner);
    return {inner, reinterpret_cast<TArray<void>*>(addr)};
}

void PropTraits<UArrayProperty>::set(const UArrayProperty* prop, uintptr_t addr, Value value) {
    auto inner = prop->read_field(&UArrayProperty::Inner);
    if (value.type != inner) {
        throw std::runtime_error("Array does not contain fields of type " + (std::string)inner->Name);
    }

    memcpy(reinterpret_cast<void*>(addr), value.base, prop->ElementSize);
}

}  // namespace unrealsdk::unreal
