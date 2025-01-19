#ifndef UNREALSDK_UNREAL_CLASSES_USTRUCT_H
#define UNREALSDK_UNREAL_CLASSES_USTRUCT_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

class UFunction;
class UProperty;

class UStruct : public UField {
   public:
    UStruct() = delete;
    UStruct(const UStruct&) = delete;
    UStruct(UStruct&&) = delete;
    UStruct& operator=(const UStruct&) = delete;
    UStruct& operator=(UStruct&&) = delete;
    ~UStruct() = delete;

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
     * things that will be destroyed by the native destructor */
    UProperty* DestructorLink;
    /** In memory only: Linked list of properties requiring post constructor initialization */
    UProperty* PostConstructLink;
    /* Array of object references embedded in script code. Mirrored for easy access by realtime
     * garbage collection code */
    TArray<UObject*> ScriptObjectReferences;
#else

#if !defined(UNREALSDK_GAME_BL1)

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

#else // defined(UNREALSDK_GAME_BL1)

   public:
    uint8_t UnknownData00[0x08];
    UField* Children;                   // 76b
    uint16_t PropertySize;              // 80b
    uint8_t UnknownData01[0x1C + 0x02]; // +2 explicit padding
    UProperty* PropertyLink;            // 112b
    uint8_t UnknownData02[0x10];
    TArray<UObject*> ScriptObjectReferences; // 132b
    uint8_t UnknownData03[0x04];

#endif

#endif

    // See the description in 'uproperty.h', we have the same issue here. `UnknownData02` is 0x10 in
    // BL2, but 0x4 in TPS. Since we need it this time, we also make provisions for setters.

    /**
     * @brief Gets the size of this class.
     *
     * @return The size of this class.
     */
    [[nodiscard]] static size_t class_size(void);

protected:
    /**
     * @brief Reads a field on a UStruct subclass, taking into account it's variable length.
     *
     * @tparam SubType The subclass of UStruct to read the field off of (should be picked up
     *                 automatically).
     * @tparam FieldType The type of the field being read (should be picked up automatically).
     * @param field Pointer to member of the field to read.
     * @return A reference to the field.
     */
    template <typename SubType,
              typename FieldType,
              typename = std::enable_if_t<std::is_base_of_v<UStruct, SubType>>>
    [[nodiscard]] const FieldType& get_field(FieldType SubType::*field) const {
#ifdef UE4
        return reinterpret_cast<const SubType*>(this)->*field;
#else
        return *reinterpret_cast<FieldType*>(
            reinterpret_cast<uintptr_t>(&(reinterpret_cast<const SubType*>(this)->*field))
            - sizeof(UStruct) + UStruct::class_size());
#endif
    }
    template <typename SubType,
              typename FieldType,
              typename = std::enable_if_t<std::is_base_of_v<UStruct, SubType>>>
    FieldType& get_field(FieldType SubType::*field) {
        return const_cast<FieldType&>(const_cast<const UStruct*>(this)->get_field(field));
    }

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)

   public:
#pragma region Iterators
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
        FieldIterator(void);
        FieldIterator(const UStruct* this_struct, UField* field);

        reference operator*() const;

        FieldIterator& operator++();
        FieldIterator operator++(int);

        bool operator==(const FieldIterator& rhs) const;
        bool operator!=(const FieldIterator& rhs) const;
    };

    struct PropertyIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = UProperty*;
        using pointer = UProperty**;
        using reference = UProperty*;

       private:
        UProperty* prop;

       public:
        PropertyIterator(void);
        PropertyIterator(UProperty* prop);

        reference operator*() const;

        PropertyIterator& operator++();
        PropertyIterator operator++(int);

        bool operator==(const PropertyIterator& rhs) const;
        bool operator!=(const PropertyIterator& rhs) const;
    };

    struct SuperFieldIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const UStruct*;
        using pointer = const UStruct**;
        using reference = const UStruct*;

       private:
        const UStruct* this_struct;

       public:
        SuperFieldIterator(void);
        SuperFieldIterator(const UStruct* this_struct);

        reference operator*() const;

        SuperFieldIterator& operator++();
        SuperFieldIterator operator++(int);

        bool operator==(const SuperFieldIterator& rhs) const;
        bool operator!=(const SuperFieldIterator& rhs) const;
    };

    /**
     * @brief Gets an iterator over this struct's fields.
     *
     * @return The iterator.
     */
    [[nodiscard]] utils::IteratorProxy<FieldIterator> fields(void) const;

    /**
     * @brief Gets an iterator over this struct's properties.
     *
     * @return The iterator.
     */
    [[nodiscard]] utils::IteratorProxy<PropertyIterator> properties(void) const;

    /**
     * @brief Gets an iterator over this struct and it's superfields.
     * @note Includes this struct itself.
     *
     * @return The iterator.
     */
    [[nodiscard]] utils::IteratorProxy<SuperFieldIterator> superfields(void) const;
#pragma endregion

    /**
     * @brief Get the actual size of the described structure, including alignment.
     *
     * @return The size which must be allocated
     */
    [[nodiscard]] size_t get_struct_size(void) const;

    /**
     * @brief Finds a child field/property by name.
     * @note Throws an exception if the child is not found.
     * @note When known to be a property, property lookup is more efficient.
     *
     * @param name The name of the child.
     * @return The found child object.
     */
    [[nodiscard]] UField* find(const FName& name) const;
    [[nodiscard]] UProperty* find_prop(const FName& name) const;

    /**
     * @brief Finds a child property/function by name, and validates that it's of the expected type.
     * @note Throws exceptions if the child is not found, or if it's of an invalid type.
     *
     * @tparam T The expected property type.
     * @param name The name of the child.
     * @return The found child object.
     */
    template <typename T>
    [[nodiscard]] T* find_prop_and_validate(const FName& name) const;
    [[nodiscard]] UFunction* find_func_and_validate(const FName& name) const;

    /**
     * @brief Checks if this structs inherits from another.
     * @note Also returns true if this struct *is* the given struct.
     *
     * @param base_struct The base struct to check if this inherits from.
     * @return True if this struct is the given struct, or inherits from it.
     */
    [[nodiscard]] bool inherits(const UStruct* base_struct) const;
};

template <>
struct ClassTraits<UStruct> {
    static inline const wchar_t* const NAME = L"Struct";
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_USTRUCT_H */
