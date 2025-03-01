#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

namespace unrealsdk::unreal {

WrappedArray::WrappedArray(const UProperty* type,
                           TArray<void>* base,
                           const UnrealPointer<void>& parent)
    : type(type), base(parent, base) {
    if (type->ArrayDim() > 1) {
        throw std::runtime_error(
            "Array has static array inner property - unsure how to handle, aborting!");
    }
}

[[nodiscard]] size_t WrappedArray::size(void) const {
    return this->base->size();
}

[[nodiscard]] size_t WrappedArray::capacity(void) const {
    return this->base->capacity();
}

void WrappedArray::reserve(size_t new_cap) const {
    if (new_cap < this->size()) {
        throw std::invalid_argument("Can't decrease array capacity below it's current size.");
    }
    this->base->reserve(new_cap, this->type->ElementSize());
}

void WrappedArray::resize(size_t new_size) {
    size_t old_size = this->base->size();
    cast(this->type, [&]<typename T>(const T* /*inner*/) {
        // Destroy any entries which will get dropped
        for (size_t idx = new_size; idx < old_size; idx++) {
            this->destroy_at<T>(idx);
        }
    });

    this->base->resize(new_size, this->type->ElementSize());

    // 0-initialize any new entries
    if (new_size > old_size) {
        auto data_ptr = reinterpret_cast<uintptr_t>(this->base->data);
        auto start = data_ptr + (old_size * this->type->ElementSize());
        auto length = (new_size - old_size) * this->type->ElementSize();
        memset(reinterpret_cast<void*>(start), 0, length);
    }
}

}  // namespace unrealsdk::unreal
