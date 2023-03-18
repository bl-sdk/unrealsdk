#include "pch.h"

#include "unreal/cast_prop.h"
#include "unreal/classes/uproperty.h"
#include "unreal/prop_traits.h"
#include "unreal/structs/fname.h"
#include "unreal/wrappers/bound_function.h"
#include "unrealsdk.h"

namespace unrealsdk::unreal {

void BoundFunction::call_with_params(void* params) const {
    static constexpr auto FUNC_NATIVE = 0x400;

    auto original_flags = this->func->FunctionFlags;
    this->func->FunctionFlags |= FUNC_NATIVE;

    process_event(this->object, this->func, params);

    func->FunctionFlags = original_flags;
}

UProperty* BoundFunction::get_next_param(UProperty* prop) {
    prop = prop->PropertyLinkNext;
    while (prop != nullptr && (prop->PropertyFlags & PROP_FLAG_PARAM) == 0) {
        prop = prop->PropertyLinkNext;
    }
    return prop;
}

void BoundFunction::validate_no_more_params(UProperty* prop) {
    for (; prop != nullptr; prop = prop->PropertyLinkNext) {
        if ((prop->PropertyFlags & PROP_FLAG_PARAM) == 0) {
            continue;
        }
        if ((prop->PropertyFlags & PROP_FLAG_RETURN) != 0) {
            continue;
        }
#ifdef UE3
        if ((prop->PropertyFlags & PROP_FLAG_OPTIONAL) != 0) {
            continue;
        }
#endif
        throw std::runtime_error("Too few parameters to function call!");
    }
}

template <>
FName cls_fname<UFunction>(void) {
    return L"Function"_fn;
}

}  // namespace unrealsdk::unreal
