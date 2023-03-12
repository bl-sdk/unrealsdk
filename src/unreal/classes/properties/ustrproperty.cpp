#include "pch.h"

#include "unreal/classes/properties/ustrproperty.h"
#include "unreal/structs/tarray.h"
#include "unreal/structs/tarray_funcs.h"
#include "unreal/structs/fstring.h"
#include "unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

PropTraits<UStrProperty>::Value PropTraits<UStrProperty>::get(const UStrProperty* /*prop*/,
                                                              uintptr_t addr) {
    return *reinterpret_cast<UnmanagedFString*>(addr);
}

void PropTraits<UStrProperty>::set(const UStrProperty* /*prop*/, uintptr_t addr, Value value) {
    *reinterpret_cast<UnmanagedFString*>(addr) = value;
}

}  // namespace unrealsdk::unreal
