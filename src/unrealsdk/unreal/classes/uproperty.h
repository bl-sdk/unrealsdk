#ifndef UNREALSDK_UNREAL_CLASSES_UPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_UPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

class UProperty : public UField {
   public:
#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
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

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    using property_flags_type = uint64_t;
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    using property_flags_type = uint32_t;
#else
#error Unknown SDK flavour
#endif

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UPROPERTY_FIELDS(X)     \
    X(int32_t, ArrayDim)                  \
    X(int32_t, ElementSize)               \
    X(property_flags_type, PropertyFlags) \
    X(int32_t, Offset_Internal)           \
    X(UProperty*, PropertyLinkNext)

    UNREALSDK_DEFINE_FIELDS_HEADER(UProperty, UNREALSDK_UPROPERTY_FIELDS);
};

template <>
struct ClassTraits<UProperty> {
    static inline const wchar_t* const NAME = L"Property";
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UPROPERTY_H */
