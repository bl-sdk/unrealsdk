#ifndef UNREALSDK_GAME_BL3_OFFSETS_UFIELD_H
#define UNREALSDK_GAME_BL3_OFFSETS_UFIELD_H

#include "unrealsdk/game/bl3/offsets/uobject.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offsets.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::game::bl3 {

using UField = unrealsdk::unreal::offsets::generic::UField<bl3::UObject>;

}

#endif

#endif /* UNREALSDK_GAME_BL3_OFFSETS_UFIELD_H */
