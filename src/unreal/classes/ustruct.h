#ifndef UNREAL_CLASSES_USTRUCT_H
#define UNREAL_CLASSES_USTRUCT_H

#include "unreal/classes/ufield.h"
#include "unreal/classes/uproperty.h"
#include "unreal/structs/tarray.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UProperty;

class UStruct : public UField {
   public:
    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

#ifdef UE4
    /* Struct this inherits from, may be null */
    UStruct* SuperField;
    /* Pointer to start of linked list of child fields */
    UField* Children;

   private:
    /* Total size of all UProperties, the allocated structure may be larger due to alignment */
    int32_t PropertySize;
    /* Alignment of structure in memory, structure will be at least this large */
    int32_t MinAlignment;
    /* Script bytecode associated with this object */
    TArray<uint8_t> Script;

   public:
    /* In memory only: Linked list of properties from most-derived to base */
    UProperty* PropertyLink;

   private:
    /* In memory only: Linked list of object reference properties from most-derived to base */
    UProperty* RefLink;
    /* In memory only: Linked list of properties requiring destruction. Note this does not include
     * things that will be destroyed byt he native destructor */
    UProperty* DestructorLink;
    /** In memory only: Linked list of properties requiring post constructor initialization */
    UProperty* PostConstructLink;
    /* Array of object references embedded in script code. Mirrored for easy access by realtime
     * garbage collection code */
    TArray<UObject*> ScriptObjectReferences;
#else
   private:
    uint8_t UnknownData00[0x8];

   public:
    UStruct* SuperField;
    UField* Children;

   private:
    uint16_t PropertySize;
    uint8_t UnknownData01[0x1A];

   public:
    UProperty* PropertyLink;

   private:
    uint8_t UnknownData02[0x10];

    TArray<UObject*> ScriptObjectReferences;
#endif

   public:
    struct FieldIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = UField*;
        using pointer = UField**;
        using reference = UField*;

       private:
        const UStruct* this_struct;
        UField* field;

       public:
        FieldIterator(const UStruct* this_struct, UField* field);

        reference operator*() const;

        FieldIterator& operator++();
        FieldIterator operator++(int);

        bool operator==(const FieldIterator& rhs) const;
        bool operator!=(const FieldIterator& rhs) const;
    };

    /**
     * @brief Get the actual size of the described structure, including alignment.
     *
     * @return The size which must be allocated
     */
    [[nodiscard]] size_t get_struct_size(void) const;

    /**
     * @brief Gets an iterator to the start of this struct's properties.
     *
     * @return The iterator.
     */
    [[nodiscard]] FieldIterator begin(void) const;

    /**
     * @brief Gets an iterator to the end of this struct's properties.
     *
     * @return The iterator.
     */
    [[nodiscard]] static FieldIterator end(void);

    /**
     * @brief Finds a child field by name.
     * @note Throws an exception if the child is not found.
     *
     * @param name The name of the field.
     * @return The found field object.
     */
    [[nodiscard]] UField* find(const FName& name) const;

    /**
     * @brief Finds a child field by name, and validates that it's of the expected type.
     * @note Throws exceptions if the child is not found, or if it's of an invalid type.
     *
     * @tparam T The expected field type.
     * @param name The name of the field.
     * @return The found field object.
     */
    template <typename T>
    [[nodiscard]] T* find_and_validate(const FName& name) const;

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_USTRUCT_H */
