#include "unreal/structs/fname.h"

namespace unrealsdk::unreal {

bool FName::operator==(const FName& other) const {
    return this->index == other.index && this->number == other.number;
}

bool FName::operator!=(const FName& other) const {
    return !operator==(other);
}

}  // namespace unrealsdk::unreal
