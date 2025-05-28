#ifndef UNREALSDK_UNREAL_STRUCTS_GOBJECTS_H
#define UNREALSDK_UNREAL_STRUCTS_GOBJECTS_H

namespace unrealsdk::unreal {

class UObject;

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

// There are a few fields we need to modify to deal with weak pointers. To be thread safe we need
// them to be atomic fields - as unreal itself does. The fields are all aligned as normal, we expect
// it to all be done in hardware.

// std::atomic isn't strictly guaranteed to be implemented like this, make sure it is.
static_assert(std::atomic<int32_t>::is_always_lock_free
                  && sizeof(std::atomic<int32_t>) == sizeof(int32_t)
                  && alignof(std::atomic<int32_t>) == alignof(int32_t),
              "atomic int32_t may not be implemented in hardware");

struct FUObjectItem {
    // NOLINTBEGIN(readability-identifier-naming)

    /// Pointer to the allocated object
    UObject* Object;
    /// Internal flags
    int32_t Flags;
    /// UObject Owner Cluster Index
    int32_t ClusterRootIndex;
    /// Weak Object Pointer Serial number associated with the object
    std::atomic<int32_t> SerialNumber;

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
    // NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)

    int32_t ObjFirstGCIndex;
    int32_t ObjLastNonGCIndex;
    int32_t MaxObjectsNotConsideredByGC;
    bool OpenForDisregardForGC;
    FChunkedFixedUObjectArray ObjObjects;

   private:
    uint8_t UnknownData00[0x178];

   public:
    std::atomic<int32_t> MasterSerialNumber;
    // NOLINTEND(readability-identifier-naming, readability-magic-numbers)
};

#endif

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_GOBJECTS_H */
