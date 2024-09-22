#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/wrappers/wrapped_multicast_delegate.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

namespace {

// Raii helper to temporarily swap out the type of a wrapped struct
struct StructTypeSwapper {
    WrappedStruct* value;
    const UStruct* original_type;

    StructTypeSwapper(WrappedStruct& value, UStruct* new_type)
        : value(&value), original_type(value.type) {
        value.type = new_type;
    }
    ~StructTypeSwapper() { this->value->type = original_type; }

    StructTypeSwapper(const StructTypeSwapper&) = delete;
    StructTypeSwapper(StructTypeSwapper&&) = delete;
    StructTypeSwapper& operator=(const StructTypeSwapper&) = delete;
    StructTypeSwapper& operator=(StructTypeSwapper&&) = delete;
};

}  // namespace

WrappedMulticastDelegate::WrappedMulticastDelegate(const UFunction* signature,
                                                   TArray<FScriptDelegate>* base,
                                                   const UnrealPointer<void>& parent)
    : signature(signature), base(parent, base) {}

void WrappedMulticastDelegate::call(WrappedStruct& params) const {
    if (params.type != this->signature) {
        throw std::runtime_error("Tried to call delegate with parameters of incorrect type: "
                                 + (std::string)params.type->Name);
    }

    if (this->base->size() == 0) {
        return;
    }

    // For the first N-1 entries, copy the params struct each time, in case the call edits it
    for (size_t i = 0; i < this->base->size() - 1; i++) {
        auto func = this->base->data[i].as_function();

        WrappedStruct params_copy = params;

        // Since the function is a different type to the signature, to do the call we need to swap
        // the type of the params with that of the exact function we're calling.
        // Previous code (or the engine itself) should have made sure this is compatible.
        const StructTypeSwapper swap{params_copy, func->func};
        func->call<void>(params_copy);
    }

    // For the last entry, we can pass the params directly
    auto func = this->base->data[this->base->size() - 1].as_function();
    const StructTypeSwapper swap{params, func->func};
    func->call<void>(params);
}

void WrappedMulticastDelegate::push_back(const BoundFunction& func) const {
    FScriptDelegate::validate_signature(func, this->signature);
    auto size = this->base->size();
    this->base->resize(size + 1);
    this->base->data[size].bind(func);
}

void WrappedMulticastDelegate::clear(void) const {
    this->base->free();
}

}  // namespace unrealsdk::unreal
