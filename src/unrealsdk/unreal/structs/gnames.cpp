#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/gnames.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(FNameEntry, UNREALSDK_FNAMEENTRY_FIELDS);

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

FNameEntry* FNamePool::at(uint32_t idx) const {
    uint32_t as_unsigned{};
    memcpy(&as_unsigned, &idx, sizeof(idx));

    // NOLINTBEGIN(readability-magic-numbers)
    const size_t name_idx = (as_unsigned & 0x0000FFFF) >> 0;
    const size_t chunk_idx = (as_unsigned & 0xFFFF000) >> 16;
    // NOLINTEND(readability-magic-numbers)

    if (chunk_idx > this->last_chunk_idx) {
        throw std::out_of_range("FName chunk index out of range");
    }
    // Assuming every name index is fine? They do seem to be 128k apart.

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    auto ptr = &(*this->chunks[chunk_idx])[name_idx];

    return reinterpret_cast<FNameEntry*>(ptr);
}

}  // namespace unrealsdk::unreal
