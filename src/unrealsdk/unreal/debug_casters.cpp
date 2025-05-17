#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl1/offsets.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/game/bl3/offsets.h"
#include "unrealsdk/game/tps/offsets.h"
#include "unrealsdk/unreal/offset_list.h"


#ifndef NDEBUG
/*
For debug builds, we define a bunch of helper functions you may call to transform the "generic"
unreal types into "concrete" ones. These are intended for use only in a debugger.

These creates a few series of global functions, named after each supported game, and "ue_generic()".
These functions have overloads for every dynamic unreal type, they take a pointer to one and return
it cast to their concret, game specific type (or to the generic one for "ue_generic()"). These
functions all also support being run as a no-op, so you don't need to double check if you have a
generic or concrete type.
*/

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

#define DEBUG_CASTER_GAMES(X, ...) \
    X(bl1, __VA_ARGS__)            \
    X(bl2, __VA_ARGS__)            \
    X(tps, __VA_ARGS__)

#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

#define DEBUG_CASTER_GAMES(X, ...) X(bl3, __VA_ARGS__)

#else
#error Unknown sdk flavour
#endif

#define DEFINE_DEBUG_CASTERS_FOR_GAME_AND_TYPE(game_name, type_name)                               \
    unrealsdk::game::game_name::type_name* game_name(unrealsdk::unreal::type_name* obj) {          \
        return reinterpret_cast<unrealsdk::game::game_name::type_name*>(obj);                      \
    }                                                                                              \
    const unrealsdk::game::game_name::type_name* game_name(                                        \
        const unrealsdk::unreal::type_name* obj) {                                                 \
        return reinterpret_cast<const unrealsdk::game::game_name::type_name*>(obj);                \
    }                                                                                              \
    unrealsdk::game::game_name::type_name* game_name(unrealsdk::game::game_name::type_name* obj) { \
        return obj;                                                                                \
    }                                                                                              \
    const unrealsdk::game::game_name::type_name* game_name(                                        \
        const unrealsdk::game::game_name::type_name* obj) {                                        \
        return obj;                                                                                \
    }                                                                                              \
    unrealsdk::unreal::type_name* ue_generic(unrealsdk::game::game_name::type_name* obj) {         \
        return reinterpret_cast<unrealsdk::unreal::type_name*>(obj);                               \
    }                                                                                              \
    const unrealsdk::unreal::type_name* ue_generic(                                                \
        const unrealsdk::game::game_name::type_name* obj) {                                        \
        return reinterpret_cast<const unrealsdk::unreal::type_name*>(obj);                         \
    }

#define DEFINE_DEBUG_GENERIC_NOOP_CASTER(type_name)                                           \
    unrealsdk::unreal::type_name* ue_generic(unrealsdk::unreal::type_name* obj) {             \
        return obj;                                                                           \
    }                                                                                         \
    const unrealsdk::unreal::type_name* ue_generic(const unrealsdk::unreal::type_name* obj) { \
        return obj;                                                                           \
    }

#define DEFINE_DEBUG_CASTERS_FOR_TYPE(type_name) \
    DEBUG_CASTER_GAMES(DEFINE_DEBUG_CASTERS_FOR_GAME_AND_TYPE, type_name)

// NOLINTBEGIN(misc-use-internal-linkage)
UNREALSDK__DYNAMIC_OFFSET_TYPES(DEFINE_DEBUG_CASTERS_FOR_TYPE)
UNREALSDK__DYNAMIC_OFFSET_TYPES(DEFINE_DEBUG_GENERIC_NOOP_CASTER)
// NOLINTEND(misc-use-internal-linkage)

// NOLINTEND(cppcoreguidelines-macro-usage)
#endif
