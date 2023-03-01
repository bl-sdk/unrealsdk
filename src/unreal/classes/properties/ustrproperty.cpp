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

    // Remove a single trailing null, if necessary
    auto size = str->size();
    if (str->data[size] == L'\0') {
        size--;
    }

    return {str->data, size};
}

void PropTraits<UStrProperty>::set(const UStrProperty* /*prop*/, uintptr_t addr, Value value) {
    auto str = reinterpret_cast<FString*>(addr);

    // Include the trailing null in the data we copy
    auto new_size = value.size() + 1;
    if (new_size >= str->capacity()) {
        str->reserve(new_size);
    }

    memcpy(str->data, value.c_str(), new_size * sizeof(*str->data));

    // Rely on `TArray::reserve` to detect overruns
    str->count = static_cast<decltype(str->count)>(new_size);
}

}  // namespace unrealsdk::unreal
