#ifndef UNREALSDK_UNREAL_WRAPPERS_GOBJECTS_H
#define UNREALSDK_UNREAL_WRAPPERS_GOBJECTS_H

#include "unrealsdk/pch.h"

#ifdef UE4
#include "unrealsdk/unreal/structs/gobjects.h"
#else
#include "unrealsdk/unreal/structs/tarray.h"
#endif

namespace unrealsdk::unreal {

class UObject;

class GObjects {
   public:
#ifdef UE4
    using internal_type = FUObjectArray*;
#else
    using internal_type = TArray<UObject*>*;
#endif
   private:
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
    GObjects(void);
    GObjects(internal_type internal);

    /**
     * @brief Get the size of the array.
     *
     * @return The size of the array.
     */
    [[nodiscard]] size_t size(void) const;

    /**
     * @brief Get an object from the array, with bounds checking.
     *
     * @param idx The index to get.
     * @return The item at that index.
     */
    [[nodiscard]] UObject* obj_at(size_t idx) const;

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

#endif /* UNREALSDK_UNREAL_WRAPPERS_GOBJECTS_H */
