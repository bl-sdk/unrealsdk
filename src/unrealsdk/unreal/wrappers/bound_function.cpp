#include "unrealsdk/pch.h"

#include "unrealsdk/exports.h"
#include "unrealsdk/locks.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

namespace func_params::impl {

UProperty* get_next_param(UProperty* prop) {
    prop = prop->PropertyLinkNext();
    while (prop != nullptr && (prop->PropertyFlags() & UProperty::PROP_FLAG_PARAM) == 0) {
        prop = prop->PropertyLinkNext();
    }
    return prop;
}

void validate_no_more_params(UProperty* prop) {
    for (; prop != nullptr; prop = prop->PropertyLinkNext()) {
        if ((prop->PropertyFlags() & UProperty::PROP_FLAG_PARAM) == 0) {
            continue;
        }
        if ((prop->PropertyFlags() & UProperty::PROP_FLAG_RETURN) != 0) {
            continue;
        }
#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
        if ((prop->PropertyFlags() & UProperty::PROP_FLAG_OPTIONAL) != 0) {
            continue;
        }
#endif

        throw std::runtime_error("Too few parameters to function call!");
    }
}

}  // namespace func_params::impl

UNREALSDK_CAPI(void, bound_function_call_with_params, const BoundFunction* self, void* params);

#ifndef UNREALSDK_IMPORTING

// This has to be implemented in the base dll so that we can lock it properly
UNREALSDK_CAPI(void, bound_function_call_with_params, const BoundFunction* self, void* params) {
    const locks::FunctionCall lock{};

    auto original_flags = self->func->FunctionFlags();
    self->func->FunctionFlags() |= UFunction::FUNC_NATIVE;

    // Calling process event itself does hold the lock, but we also need to guard messing with the
    // function flags
    unrealsdk::internal::process_event(self->object, self->func, params);

    self->func->FunctionFlags() = original_flags;
}

#endif

void BoundFunction::call_with_params(void* params) const {
    UNREALSDK_MANGLE(bound_function_call_with_params)(this, params);
}

}  // namespace unrealsdk::unreal
