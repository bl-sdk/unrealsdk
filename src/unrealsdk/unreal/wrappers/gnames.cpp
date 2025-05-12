#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/gnames.h"
#include "unrealsdk/unreal/wrappers/gnames.h"

namespace unrealsdk::unreal {

GNames::GNames(void) : internal(nullptr) {}
GNames::GNames(internal_type internal) : internal(internal) {}

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

size_t GNames::size(void) const {
    return this->internal->Count;
}

FNameEntry* GNames::at(size_t idx) const {
    if (std::cmp_greater_equal(idx, this->internal->Count)) {
        throw std::out_of_range("GObjects index out of range");
    }
    return this->internal->at(idx);
}

#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

size_t GNames::size(void) const {
    return this->internal->size();
}

FNameEntry* GNames::at(size_t idx) const {
    return this->internal->at(idx);
}

#else
#error Unknown SDK flavour
#endif

}  // namespace unrealsdk::unreal
