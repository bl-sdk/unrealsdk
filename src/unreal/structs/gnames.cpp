#include "pch.h"

#include "unreal/structs/gnames.h"

namespace unrealsdk::unreal {

bool FNameEntry::is_wide(void) const {
    return (this->Index & NAME_WIDE_MASK) != 0;
}

#ifdef UE4

FNameEntry* FChunkedFNameEntryArray::at(size_t idx) const {
    if (idx > this->Count) {
        throw std::out_of_range("FChunkedFNameEntryArray index out of range");
    }
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,
    //             cppcoreguidelines-pro-bounds-constant-array-index)
    return this->Objects[idx / this->NumElementsPerChunk][idx % this->NumElementsPerChunk];
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,
    //           cppcoreguidelines-pro-bounds-constant-array-index)
};

#endif

}  // namespace unrealsdk::unreal
