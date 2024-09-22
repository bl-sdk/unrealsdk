#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::unreal {

#ifdef UE3

UObject* FScriptDelegate::get_object(void) const {
    return this->object;
}

void FScriptDelegate::set_object(UObject* obj) {
    this->object = obj;
}

#else

UObject* FScriptDelegate::get_object(void) const {
    return unrealsdk::gobjects().get_weak_object(&this->object);
}

void FScriptDelegate::set_object(UObject* obj) {
    unrealsdk::gobjects().set_weak_object(&this->object, obj);
}
#endif

[[nodiscard]] std::optional<BoundFunction> FScriptDelegate::as_function(void) const {
    auto obj = this->get_object();

    if (obj == nullptr) {
        return std::nullopt;
    }

    return BoundFunction{.func = obj->Class->find_func_and_validate(this->func_name),
                         .object = obj};
}

void FScriptDelegate::bind(const std::optional<BoundFunction>& func) {
    if (!func.has_value()) {
        this->set_object(nullptr);
        this->func_name = FName{0, 0};
        return;
    }

    this->set_object(func->object);
    this->func_name = func->func->Name;
}

void FScriptDelegate::validate_signature(const std::optional<BoundFunction>& func,
                                         const UFunction* signature) {
    // Removing a delegate is always valid
    if (!func.has_value()) {
        return;
    }

    // Since delegates store the function name, make sure we can find this function again from just
    // it's name
    {
        UFunction* func_from_find = nullptr;
        try {
            func_from_find = func->object->Class->find_func_and_validate(func->func->Name);
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument(unrealsdk::fmt::format(
                "Could not convert function to delegate: could not find function with name '{}'",
                func->func->Name));
        }
        if (func_from_find != func->func) {
            throw std::invalid_argument(utils::narrow(unrealsdk::fmt::format(
                L"Could not convert function to delegate: got another function with the same name,"
                "{} instead of {}",
                func_from_find->get_path_name(), func->func->get_path_name())));
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
        auto func_props = func->func->properties();
        auto sig_props = signature->properties();

        auto [func_diff, sig_diff] = std::ranges::mismatch(
            func_props, sig_props,
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
}

}  // namespace unrealsdk::unreal
