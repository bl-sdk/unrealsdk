#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/gnames.h"
#include "unrealsdk/unreal/wrappers/gnames.h"

namespace unrealsdk::unreal {

GNames::GNames(void) : internal(nullptr) {}
GNames::GNames(internal_type internal) : internal(internal) {}

#if defined(UE4)

size_t GNames::size(void) const {
    return this->internal->Count;
}

FNameEntry* GNames::at(size_t idx) const {
    if (idx > (size_t)this->internal->Count) {
        throw std::out_of_range("GObjects index out of range");
    }
    return this->internal->at(idx);
}

#elif defined(UE3)

size_t GNames::size(void) const {
    return this->internal->size();
}

FNameEntry* GNames::at(size_t idx) const {
    return this->internal->at(idx);
}

#else
#error Unknown UE version
#endif

}  // namespace unrealsdk::unreal
