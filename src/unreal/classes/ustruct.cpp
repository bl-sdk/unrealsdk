#include "pch.h"

#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

size_t UStruct::get_struct_size(void) const {
#ifdef UE4
    return (this->PropertySize + this->MinAlignment - 1) & ~(this->MinAlignment - 1);
#else
    return this->PropertySize;
#endif
}

}  // namespace unrealsdk::unreal
