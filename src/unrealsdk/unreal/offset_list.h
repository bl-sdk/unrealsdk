#ifndef UNREALSDK_UNREAL_OFFSET_LIST_H
#define UNREALSDK_UNREAL_OFFSET_LIST_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/attribute_property.h"
#include "unrealsdk/unreal/classes/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"

namespace unrealsdk::unreal::offsets {

// Since this type is shared between dlls, also force consistent padding between compilers, even
// though this isn't an unreal type
#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UNREALSDK__DYNAMIC_OFFSET_TYPES(X) \
    X(UObject)                             \
    X(UField)                              \
    X(UStruct)                             \
    X(UProperty)                           \
    X(UClass)                              \
    X(UScriptStruct)                       \
    X(UFunction)                           \
    X(UConst)                              \
    X(UByteProperty)                       \
    X(UByteAttributeProperty)              \
    X(UFloatAttributeProperty)             \
    X(UIntAttributeProperty)               \
    X(UObjectProperty)                     \
    X(UClassProperty)                      \
    X(UInterfaceProperty)                  \
    X(USoftClassProperty)

#define UNREALSDK_OFFSETS__DEFINE_OFFSET_LIST_MEMBERS(name) unrealsdk::unreal::name::Offsets name;
#define UNREALSDK_OFFSETS__NESTED_FROM_TYPE(name) \
    unrealsdk::unreal::name::Offsets::from<typename T::name>(),
#define UNREALSDK_OFFSETS__NESTED_FROM_NAMESPACE(name) \
    unrealsdk::unreal::name::Offsets::from<name>(),

// NOLINTEND(cppcoreguidelines-macro-usage)

struct OffsetList {
    // NOLINTNEXTLINE(readability-identifier-naming)
    UNREALSDK__DYNAMIC_OFFSET_TYPES(UNREALSDK_OFFSETS__DEFINE_OFFSET_LIST_MEMBERS)

    /**
     * @brief Creates a full offset list based off of the templated type.
     * @note Assumes all the game specific types are subtypes - i.e. `T::UObject` is valid.
     *
     * @tparam T The templated type holding all the game-specific types.
     * @return A new offset list.
     */
    template <typename T>
    static constexpr OffsetList from(void) {
        return {UNREALSDK__DYNAMIC_OFFSET_TYPES(UNREALSDK_OFFSETS__NESTED_FROM_TYPE)};
    }
};

/**
 * @brief Macro to create a full offset list within the current namespace.
 * @note Assumes all the game specific types are directly accessible - i.e just `UObject` is valid.
 *
 * @return A new offset list.
 */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OFFSET_LIST_FROM_NAMESPACE()                                              \
    unrealsdk::unreal::offsets::OffsetList {                                      \
        UNREALSDK__DYNAMIC_OFFSET_TYPES(UNREALSDK_OFFSETS__NESTED_FROM_NAMESPACE) \
    }

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal::offsets

#endif /* UNREALSDK_UNREAL_OFFSET_LIST_H */
