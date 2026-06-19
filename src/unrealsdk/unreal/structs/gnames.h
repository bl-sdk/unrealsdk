#ifndef UNREALSDK_UNREAL_STRUCTS_GNAMES_H
#define UNREALSDK_UNREAL_STRUCTS_GNAMES_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/offsets.h"

namespace unrealsdk::unreal {

// NOLINTNEXTLINE(readability-identifier-naming)
struct FNameEntry {
    static constexpr auto NAME_SIZE = 1024;
    static constexpr auto NAME_WIDE_MASK = 0x1;

    // NOLINTBEGIN(readability-identifier-naming)
    using name_union = union {
        char Ansi[NAME_SIZE];
        wchar_t Wide[NAME_SIZE];
    };
    // NOLINTEND(readability-identifier-naming)

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_FNAMEENTRY_FIELDS(X) \
    X(name_union, Name)                \
    X(uint8_t, Flags)

    UNREALSDK_DEFINE_FIELDS_HEADER(FNameEntry, UNREALSDK_FNAMEENTRY_FIELDS);
};

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

// NOLINTNEXTLINE(readability-identifier-naming)
struct TStaticIndirectArrayThreadSafeRead_FNameEntry {
    // NOLINTBEGIN(readability-identifier-naming)
    enum {
        MaxTotalElements = 4 * 1024 * 1024,
        ElementsPerChunk = 16384,
        ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
    };

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    FNameEntry** Objects[ChunkTableSize];
    int32_t Count;
    int32_t ChunksCount;
    // NOLINTEND(readability-identifier-naming)

    /**
     * @brief Get an element in the array, with bounds checking.
     *
     * @param idx The index in the array to get.
     * @return The entry at that index.
     */
    [[nodiscard]] FNameEntry* at(size_t idx) const;
};

struct FNamePool {
    // NOLINTNEXTLINE(readability-magic-numbers)
    uint8_t unknown[0x8];

    uint32_t last_chunk_idx;
    uint32_t num_entries;

    // We treat these as variable length arrays.
    using chunk = wchar_t[1];
    chunk* chunks[1];

    /**
     * @brief Get an element in the array, with bounds checking.
     *
     * @param idx The fname index to get.
     * @return The item at that index.
     */
    [[nodiscard]] FNameEntry* at(uint32_t idx) const;
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_GNAMES_H */
