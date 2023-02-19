#ifndef UNREAL_STRUCTS_TARRAY_FUNCS_H
#define UNREAL_STRUCTS_TARRAY_FUNCS_H

#include "pch.h"

#include "game/game_hook.h"
#include "unreal/structs/tarray.h"

namespace unrealsdk::unreal {

template <typename T>
void TArray<T>::reserve(size_t new_cap, size_t element_size) {
    if (new_cap > MAX_CAPACITY) {
        throw std::length_error("Tried to increase TArray beyond max capacity!");
    }

    size_t new_size = new_cap * element_size;

    /*
    If realloc fails, it'll return null, which we want to handle "gracefully" by throwing on our
     thread and letting the game continue on as if nothing happened.
    We have a race condition between the realloc call finishing and us overwriting the data pointer
     however, another thread could try access the now free'd memory.
    Since the realloc failure should be a lot rarer, overwrite the pointer ASAP, and only check if
     it's valid after.
    */
    auto old_data = this->data;
    this->data = game::realloc<T>(this->data, new_size);
    if (this->data == nullptr) {
        this->data = old_data;
        throw std::runtime_error("Failed to allocate memory to resize array!");
    }

    this->max = new_cap;
}

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_TARRAY_FUNCS_H */
