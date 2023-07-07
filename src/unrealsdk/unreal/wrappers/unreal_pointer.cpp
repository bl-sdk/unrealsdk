#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal::impl {

size_t UnrealPointerControl::inc_ref(void) {
    if (this->refs == std::numeric_limits<size_t>::max()) {
        throw std::runtime_error("Unreal smart pointer reached maximum references!");
    }
    return ++this->refs;
}

size_t UnrealPointerControl::dec_ref(void) {
    if (this->refs == 0) {
        throw std::runtime_error(
            "Tried to decrement reference from unreal smart pointer already at zero!");
    }
    return --this->refs;
}

}  // namespace unrealsdk::unreal::impl
