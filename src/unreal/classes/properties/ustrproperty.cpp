#include "pch.h"

#include "unreal/classes/properties/ustrproperty.h"
#include "unreal/structs/tarray.h"
#include "unreal/structs/tarray_funcs.h"
#include "unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

using FString = TArray<wchar_t>;

PropTraits<UStrProperty>::Value PropTraits<UStrProperty>::get(const UStrProperty* /*prop*/,
                                                              uintptr_t addr) {
    auto str = reinterpret_cast<FString*>(addr);
    return {str->data, str->size()};
}

void PropTraits<UStrProperty>::set(const UStrProperty* /*prop*/, uintptr_t addr, Value value) {
    auto str = reinterpret_cast<FString*>(addr);

    auto new_size = value.size();
    if (new_size >= str->capacity()) {
        str->reserve(new_size + 1);
    }

    memcpy(str->data, value.data(), new_size * sizeof(*str->data));
    str->data[new_size] = '\0';

    // Rely on `TArray::reserve` to detect overruns
    str->count = static_cast<decltype(str->count)>(new_size);
}

}  // namespace unrealsdk::unreal
