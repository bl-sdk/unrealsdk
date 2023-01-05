#ifndef UNREAL_GOBJECTS_H
#define UNREAL_GOBJECTS_H

#include "pch.h"

#include "unreal/classes/uobject.h"

#ifdef UE4
#include "unreal/structs/gobjects.h"
#else
#include "unreal/structs/tarray.h"
#endif

namespace unrealsdk::unreal {

/**
 * @brief GObjects wrapper class
 */
class GObjects {
   private:
#ifdef UE4
    using internal_type = FUObjectArray*;
#else
    using internal_type = TArray<UObject*>*;
#endif

    internal_type internal;

   public:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = UObject*;
        using pointer = UObject**;
        using reference = UObject*;

       private:
        const GObjects& gobjects;
        size_t idx;

       public:
        Iterator(const GObjects& gobjects, size_t idx);

        reference operator*() const;

        Iterator& operator++();
        Iterator operator++(int);

        bool operator==(const Iterator& rhs) const;
        bool operator!=(const Iterator& rhs) const;
    };

    /**
     * @brief Construct a new GObjects wrapper.
     *
     * @param internal A pointer to the internal GObjects structure.
     */
    GObjects(internal_type internal);

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
    [[nodiscard]] UObject* at(size_t idx) const;

    /**
     * @brief Gets an iterator to the start of GObjects.
     *
     * @return The iterator.
     */
    [[nodiscard]] Iterator begin(void) const;

    /**
     * @brief Gets an iterator to the end of GObjects.
     *
     * @return The iterator.
     */
    [[nodiscard]] Iterator end(void) const;
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_GOBJECTS_H */
