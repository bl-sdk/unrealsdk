#include "pch.h"

#include "unreal/cast_prop.h"
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

void BoundFunction::free_params(void* params) const {
    for (const auto& prop : this->func->properties()) {
        cast_prop(prop, [params]<typename T>(const T* prop) {
            destroy_property<T>(prop, 0, reinterpret_cast<uintptr_t>(params));
        });
    }
}

void BoundFunction::validate_no_more_params(UProperty* prop) {
#ifdef UE3
    static constexpr auto PROP_FLAG_OPTIONAL = 0x10;
#endif

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
