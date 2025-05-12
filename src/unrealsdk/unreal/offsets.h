#ifndef UNREALSDK_UNREAL_OFFSETS_H
#define UNREALSDK_UNREAL_OFFSETS_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal::offsets {

using offset_type = uint16_t;

template <typename From, typename To>
using copy_cv = std::conditional_t<
    std::is_const_v<From>,
    std::add_const_t<std::conditional_t<std::is_volatile_v<From>, std::add_volatile_t<To>, To>>,
    /* align      */ std::conditional_t<std::is_volatile_v<From>, std::add_volatile_t<To>, To>>;

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// Internal macros
#define UNREALSDK_OFFSETS__DEFINE_OFFSET_MEMBERS(type, name) \
    unrealsdk::unreal::offsets::offset_type name;
#define UNREALSDK_OFFSETS__OFFSETOF_ASSERTS(type, name) \
    static_assert(offsetof(T, name)                     \
                  < std::numeric_limits<unrealsdk::unreal::offsets::offset_type>::max());
#define UNREALSDK_OFFSETS__OFFSETOF(type, name) \
    static_cast<unrealsdk::unreal::offsets::offset_type>(offsetof(T, name)),
#define UNREALSDK_OFFSETS__DEFINE_GETTER(type, name)                                 \
    template <typename T>                                                            \
    [[nodiscard]] unrealsdk::unreal::offsets::copy_cv<T, type>& name(this T& self) { \
        return *reinterpret_cast<unrealsdk::unreal::offsets::copy_cv<T, type>*>(     \
            reinterpret_cast<uintptr_t>(&self) + Offsets::get(&Offsets::name));      \
    }

#if defined(__MINGW32__) || defined(__clang__)
#define UNREALSDK_OFFSETS__OFFSETOF_PRAGMA_PUSH \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Winvalid-offsetof\"")
#define UNREALSDK_OFFSETS__OFFSETOF_PRAGMA_POP _Pragma("GCC diagnostic pop")

#else
#define UNREALSDK_OFFSETS__OFFSETOF_PRAGMA_PUSH
#define UNREALSDK_OFFSETS__OFFSETOF_PRAGMA_POP
#endif

/**
 * @brief Header file macro to defines all the machinery for variable offset fields.
 * @note Should be placed within the class definition.
 *
 * @param ClassName The name of the class these fields are being defined for.
 * @param X_MACRO An X macro list of the fields to define. See the example below.
 */
#define UNREALSDK_DEFINE_FIELDS_HEADER(ClassName, X_MACRO)             \
   public:                                                             \
    struct Offsets;                                                    \
    /* NOLINTNEXTLINE(readability-identifier-naming) */                \
    X_MACRO(UNREALSDK_OFFSETS__DEFINE_GETTER)                          \
    struct Offsets {                                                   \
        /* NOLINTNEXTLINE(readability-identifier-naming) */            \
        X_MACRO(UNREALSDK_OFFSETS__DEFINE_OFFSET_MEMBERS)              \
        template <typename T>                                          \
        static constexpr Offsets from() {                              \
            UNREALSDK_OFFSETS__OFFSETOF_PRAGMA_PUSH                    \
            X_MACRO(UNREALSDK_OFFSETS__OFFSETOF_ASSERTS);              \
            return {X_MACRO(UNREALSDK_OFFSETS__OFFSETOF)};             \
            UNREALSDK_OFFSETS__OFFSETOF_PRAGMA_POP                     \
        }                                                              \
        static unrealsdk::unreal::offsets::offset_type get(            \
            unrealsdk::unreal::offsets::offset_type Offsets::* field); \
    }  // deliberately no semicolon - forward declared earlier so that we could put this last

/**
 * @brief Source file macro to defines all the machinery for variable offset fields.
 * @note Should be placed at the top level (but within the relevant namespace).
 *
 * @param ClassName The name of the class these fields are being defined for.
 * @param X_MACRO An X macro list of the fields to define. See the example below.
 */
#define UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(ClassName, X_MACRO)                \
    unrealsdk::unreal::offsets::offset_type ClassName::Offsets::get(           \
        unrealsdk::unreal::offsets::offset_type ClassName::Offsets::* field) { \
        return unrealsdk::internal::get_offsets().ClassName.*field;            \
    }

#if 0  // NOLINT(readability-avoid-unconditional-preprocessor-if)

// The above macros take an X macro field list. This is a macro function, which takes another
// macro function, and calls it for each field

#define UOBJECT_FIELDS(X) \
    X(UClass*, Class)     \
    X(UObject*, Outer)

// The inner macro always takes the following args:
// - The field's type
// - The field's name

// Calling the macro
UNREALSDK_DEFINE_FIELDS_HEADER(UObject, UOBJECT_FIELDS);

// Creates a class approximately like the following:
struct Offsets {
    offset_type Class;
    offset_type Outer;

    template <typename T>
    static constexpr Offsets from() {
        return {
            offsetof(T, Class),
            offsetof(T, Outer),
        };
    }
};

// You can use a one-liner to fill this with all the relevant offsets from another type
auto bl2_offsets = Offsets::from<bl2::UObject>();

// The macro also creates a bunch of getters like the following
template <typename T>
[[nodiscard]] copy_cv<T, UClass*>& Class(this T& self) {
    return *reinterpret_cast<copy_cv<T, UClass*>*>(reinterpret_cast<uintptr_t>(&self)
                                                   + Offsets::get(&Offsets::Class));
}

// Since these return a reference, they can be used pretty much the exact same way as a member.
auto cls = obj->Class()->find(L"MyField"_fn);
obj->Class() = some_other_class;

// Storing the reference returned from these functions has the exact same semantics as taking a
// member reference - i.e. don't do it, it's only valid for the parent object's lifetime.

#endif

// NOLINTEND(cppcoreguidelines-macro-usage)

}  // namespace unrealsdk::unreal::offsets

#endif /* UNREALSDK_UNREAL_OFFSETS_H */
