#ifndef UNREAL_STRUCTS_GOBJECTS_H
#define UNREAL_STRUCTS_GOBJECTS_H

namespace unrealsdk::unreal {

#if defined(UE4)

struct FUObjectItem {
    // NOLINTBEGIN(readability-identifier-naming)

    /// Pointer to the allocated object
    class UObject* Object;
    /// Internal flags
    uint32_t Flags;
    /// UObject Owner Cluster Index
    uint32_t ClusterRootIndex;
    /// Weak Object Pointer Serial number associated with the object
    uint32_t SerialNumber;

    // NOLINTEND(readability-identifier-naming)
};

struct FChunkedFixedUObjectArray {
    // NOLINTBEGIN(readability-identifier-naming)

    enum {
        NumElementsPerChunk = 64 * 1024,
    };

    /** Master table to chunks of pointers **/
    FUObjectItem** Objects;
    /** If requested, a contiguous memory where all objects are allocated **/
    FUObjectItem* PreAllocatedObjects;
    /** Maximum number of elements **/
    int32_t Max;
    /** Number of elements we currently have **/
    int32_t Count;
    /** Maximum number of chunks **/
    int32_t MaxChunks;
    /** Number of chunks we currently have **/
    int32_t NumChunks;

    // NOLINTEND(readability-identifier-naming)

    /**
     * @brief Get an element in the array, with bounds checking.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    [[nodiscard]] FUObjectItem* at(size_t idx) const;
};

struct FUObjectArray {
    // NOLINTBEGIN(readability-identifier-naming)

    int32_t ObjFirstGCIndex;
    int32_t ObjLastNonGCIndex;
    int32_t MaxObjectsNotConsideredByGC;
    bool OpenForDisregardForGC;
    FChunkedFixedUObjectArray ObjObjects;

    // NOLINTEND(readability-identifier-naming)
};

#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_GOBJECTS_H */
