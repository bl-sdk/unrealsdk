#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

namespace unrealsdk::unreal {

WrappedArray::WrappedArray(const UProperty* type,
                           TArray<void>* base,
                           const UnrealPointer<void>& parent)
    : type(type), base(parent, base) {
    if (type->ArrayDim > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }
}

[[nodiscard]] size_t WrappedArray::size(void) const {
    return this->base->size();
}

void WrappedArray::reserve(size_t new_cap) const {
    return this->base->reserve(new_cap, this->type->ElementSize);
}

void WrappedArray::resize(size_t new_size) const {
    return this->base->resize(new_size, this->type->ElementSize);
}

}  // namespace unrealsdk::unreal
