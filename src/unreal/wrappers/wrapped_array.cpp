#include "pch.h"

#include "unreal/wrappers/wrapped_array.h"

namespace unrealsdk::unreal {

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
