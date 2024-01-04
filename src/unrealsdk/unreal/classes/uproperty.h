#ifndef UNREALSDK_UNREAL_CLASSES_UPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_UPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

/*
There is a massive issue with `UProperty`.

We've observed that it's 0x80 bytes big in BL2, but it's only 0x74 in TPS
These are both 32-bit UE3 games, so we want to support them both from the same dll.

The extra 0xC bytes are placed right at the end, which makes accesses within `UProperty` itself
 simple enough, especially since we don't care about whatever fields they are.
The issue is subclasses. We very much do care about fields on subclasses, which will be placed
 after the variable offset.
Luckily, we can read the size off of `Core.Property` at runtime, and do some maths to work out the
 actual offsets of the fields we want, using the helper function `read_field`.

This means:
- All accesses to UProperty subclasses' fields **MUST** be done through `read_field`.
- All accesses to base UProperty fields **MUST** be direct, and **MUST NOT** use `read_field`
*/

class UProperty : public UField {
   public:
#ifdef UE3
    static constexpr auto PROP_FLAG_OPTIONAL = 0x10;
#endif
    static constexpr auto PROP_FLAG_PARAM = 0x80;
    static constexpr auto PROP_FLAG_OUT = 0x100;
    static constexpr auto PROP_FLAG_RETURN = 0x400;

    UProperty() = delete;
    UProperty(const UProperty&) = delete;
    UProperty(UProperty&&) = delete;
    UProperty& operator=(const UProperty&) = delete;
    UProperty& operator=(UProperty&&) = delete;
    ~UProperty() = delete;

    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

#ifdef UE4
    int32_t ArrayDim;
    int32_t ElementSize;
    uint64_t PropertyFlags;

   private:
    uint16_t RepIndex;
    uint8_t BlueprintReplicationCondition;

   public:
    int32_t Offset_Internal;

   private:
    FName RepNotifyFunc;

   public:
    /** In memory only: Linked list of properties from most-derived to base **/
    UProperty* PropertyLinkNext;

   private:
    /** In memory only: Linked list of object reference properties from most-derived to base **/
    UProperty* NextRef;
    /** In memory only: Linked list of properties requiring destruction. Note this does not include
     * things that will be destroyed byt he native destructor **/
    UProperty* DestructorLinkNext;
    /** In memory only: Linked list of properties requiring post constructor initialization.**/
    UProperty* PostConstructLinkNext;  // 0x0030(0x0040) MISSED OFFSET
#else

    int32_t ArrayDim;
    int32_t ElementSize;
    uint32_t PropertyFlags;

   private:
    uint8_t UnknownData00[0x14];

   public:
    int32_t Offset_Internal;
    UProperty* PropertyLinkNext;

   private:
    uint8_t UnknownData01[0x18];

    /**
     * @brief Gets the size of this class.
     *
     * @return The size of this class.
     */
    [[nodiscard]] static size_t class_size(void);

#endif
   public:
    /**
     * @brief Reads a field on a UProperty subclass, taking into account it's variable length.
     *
     * @tparam PropertyType The subclass of UProperty to read the field off of (should be picked up
     *                      automatically).
     * @tparam FieldType The type of the field being read (should be picked up automatically).
     * @param prop The property to read the field off of.
     * @param field Pointer to member of the field to read.
     * @return The field's value.
     */
    template <typename PropertyType,
              typename FieldType,
              typename = std::enable_if_t<std::is_base_of_v<UProperty, PropertyType>>>
    FieldType read_field(FieldType PropertyType::*field) const {
#ifdef UE4
        return reinterpret_cast<const PropertyType*>(this)->*field;
#else
        return *reinterpret_cast<FieldType*>(
            reinterpret_cast<uintptr_t>(&(reinterpret_cast<const PropertyType*>(this)->*field))
            - sizeof(UProperty) + UProperty::class_size());
#endif
    }

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
};

template <>
struct ClassTraits<UProperty> {
    static inline const wchar_t* const NAME = L"Property";
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UPROPERTY_H */
