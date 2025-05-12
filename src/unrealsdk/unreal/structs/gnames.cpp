#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/gnames.h"

namespace unrealsdk::unreal {

bool FNameEntry::is_wide(void) const {
    return (this->Index & NAME_WIDE_MASK) != 0;
}

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

FNameEntry* TStaticIndirectArrayThreadSafeRead_FNameEntry::at(size_t idx) const {
    if (std::cmp_greater_equal(idx, this->Count)) {
        throw std::out_of_range("FNameEntry index out of range");
    }
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,
    //             cppcoreguidelines-pro-bounds-constant-array-index)
    return this->Objects[idx / this->ElementsPerChunk][idx % this->ElementsPerChunk];
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,
    //           cppcoreguidelines-pro-bounds-constant-array-index)
};

#endif

}  // namespace unrealsdk::unreal
