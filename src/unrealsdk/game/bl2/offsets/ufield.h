#ifndef UNREALSDK_GAME_BL2_OFFSETS_UFIELD_H
#define UNREALSDK_GAME_BL2_OFFSETS_UFIELD_H

#include "unrealsdk/game/bl2/offsets/uobject.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offsets.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::game::bl2 {

using UField = unrealsdk::unreal::offsets::generic::UField<bl2::UObject>;

}

#endif

#endif /* UNREALSDK_GAME_BL2_OFFSETS_UFIELD_H */
