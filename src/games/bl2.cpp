#include "pch.h"

#include "games/bl2.h"
#include "games/game_hook.h"
#include "sigscan.h"
#include "unreal/gobjects.h"

using namespace unrealsdk::sigscan;

namespace unrealsdk::games {

BL2Hook::BL2Hook(void) {
    LOG(MISC, "Hooking as BL2");

    auto [start, size] = get_exe_range();

    auto gobjects_ptr = *scan<unreal::GObjects::internal_type*>(start, size, this->gobjects_sig);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);
    this->gobjects = unreal::GObjects(gobjects_ptr);
}

}  // namespace unrealsdk::games
