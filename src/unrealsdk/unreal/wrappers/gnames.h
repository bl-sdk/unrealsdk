#ifndef UNREALSDK_UNREAL_WRAPPERS_GNAMES_H
#define UNREALSDK_UNREAL_WRAPPERS_GNAMES_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/gnames.h"

#ifdef UE3
#include "unrealsdk/unreal/structs/tarray.h"
#endif

namespace unrealsdk::unreal {

class GNames {
   public:
#ifdef UE4
    using internal_type = TStaticIndirectArrayThreadSafeRead_FNameEntry*;
#else
    using internal_type = TArray<FNameEntry*>*;
#endif
   private:
    internal_type internal;

   public:
    /**
     * @brief Construct a new GNames wrapper.
     *
     * @param internal A pointer to the internal GNames structure.
     */
    GNames(void);
    GNames(internal_type internal);

    /**
     * @brief Get the size of the array.
     *
     * @return The size of the array.
     */
    [[nodiscard]] size_t size(void) const;

    /**
     * @brief Get an element in the array, with bounds checking.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    [[nodiscard]] FNameEntry* at(size_t idx) const;
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_WRAPPERS_GNAMES_H */
