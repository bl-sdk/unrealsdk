#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/udelegateproperty.h"
#include "unrealsdk/format.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

UFunction* UDelegateProperty::get_signature(void) const {
    return this->read_field(&UDelegateProperty::Signature);
}

PropTraits<UDelegateProperty>::Value PropTraits<UDelegateProperty>::get(
    const UDelegateProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto delegate = reinterpret_cast<FScriptDelegate*>(addr);
    auto object = delegate->get_object();

    if (object == nullptr) {
        return std::nullopt;
    }

    return BoundFunction{object->Class->find_func_and_validate(delegate->func_name), object};
}

void PropTraits<UDelegateProperty>::set(const UDelegateProperty* prop,
                                        uintptr_t addr,
                                        const Value& value) {
    auto delegate = reinterpret_cast<FScriptDelegate*>(addr);

    if (!value.has_value()) {
        delegate->set_object(nullptr);
        delegate->func_name = FName{0, 0};
        return;
    }

    // Since delegates store the function name, make sure we can find this function again from just
    // it's name
    {
        UFunction* func_from_find = nullptr;
        try {
            func_from_find = value->object->Class->find_func_and_validate(value->func->Name);
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument(unrealsdk::fmt::format(
                "Could not convert function to delegate: could not find function with name '{}'",
                value->func->Name));
        }
        if (func_from_find != value->func) {
            throw std::invalid_argument(unrealsdk::fmt::format(
                "Could not convert function to delegate: got another function with the same name,"
                "{} instead of {}",
                func_from_find->get_path_name(), value->func->get_path_name()));
        }
    }

    /*
    Perform some basic signature validation.

    I think when developing the game, this is actually handled by the unrealscript/blueprint/c++
    compilers - which makes it difficult to find how it should work in the executable.

    Experimentation has shown "natural" delegates seem to always have the same types of properties
    in the same order as the signature function - so just copying that for now.

    You can quite easily trick this - e.g. two object properties of incompatible types still get
    marked as ok - but it should be enough to catch stupid, obvious, mistakes, while still being
    reasonably simple to implement.
    */
    {
        auto func_props = value->func->properties();
        auto sig_props = prop->get_signature()->properties();

        auto [func_diff, sig_diff] = std::mismatch(
            func_props.begin(), func_props.end(), sig_props.begin(), sig_props.end(),
            [](UProperty* func, UProperty* sig) { return func->Class == sig->Class; });

        if (func_diff != func_props.end() && sig_diff != sig_props.end()) {
            throw std::invalid_argument(unrealsdk::fmt::format(
                "Function signature does not match delegate: function's {} {} != delegate's {} {}",
                (*func_diff)->Class->Name, (*func_diff)->Name, (*sig_diff)->Class->Name,
                (*sig_diff)->Name));
        }
        if (func_diff != func_props.end() && sig_diff == sig_props.end()) {
            throw std::invalid_argument(
                "Function signature does not match delegate: function has too many args");
        }
        if (func_diff == func_props.end() && sig_diff != sig_props.end()) {
            throw std::invalid_argument(
                "Function signature does not match delegate: function has too few args");
        }
    }

    delegate->set_object(value->object);
    delegate->func_name = value->func->Name;
}

}  // namespace unrealsdk::unreal
