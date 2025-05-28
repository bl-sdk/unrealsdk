#ifndef UNREALSDK_UNREAL_STRUCTS_GNAMES_H
#define UNREALSDK_UNREAL_STRUCTS_GNAMES_H

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

struct FNameEntry {
    static constexpr auto NAME_SIZE = 1024;
    static constexpr auto NAME_WIDE_MASK = 0x1;
    static constexpr auto NAME_INDEX_SHIFT = 1;

    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    int32_t Index;

   private:
    uint8_t UnknownData00[0x04];

   public:
    FNameEntry* HashNext;
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
   private:
    uint8_t UnknownData00[0x08];

   public:
    int32_t Index;

   private:
    uint8_t UnknownData01[0x04];

   public:
#else
#error Unknown SDK flavour
#endif

    union {
        char AnsiName[NAME_SIZE];
        wchar_t WideName[NAME_SIZE];
    };

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)

    /**
     * @brief Checks if this entry holds a wide string.
     *
     * @return True if this entry is wide, false if it's ANSI.
     */
    [[nodiscard]] bool is_wide(void) const;
};

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

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
     * @param idx The index to get.
     * @return The item at that index.
     */
    [[nodiscard]] FNameEntry* at(size_t idx) const;
};

#endif

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_GNAMES_H */
