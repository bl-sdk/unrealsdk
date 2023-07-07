#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

PropTraits<UStrProperty>::Value PropTraits<UStrProperty>::get(
    const UStrProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    return *reinterpret_cast<UnmanagedFString*>(addr);
}

void PropTraits<UStrProperty>::set(const UStrProperty* /*prop*/,
                                   uintptr_t addr,
                                   const Value& value) {
    *reinterpret_cast<UnmanagedFString*>(addr) = value;
}

void PropTraits<UStrProperty>::destroy(const UStrProperty* /*prop*/, uintptr_t addr) {
    auto fstr = reinterpret_cast<UnmanagedFString*>(addr);
    u_free(fstr->data);
    fstr->data = nullptr;
}

}  // namespace unrealsdk::unreal
