#include "pch.h"

#include "games/bl3.h"
#include "games/game_hook.h"
#include "sigscan.h"
#include "unreal/gobjects.h"

using namespace unrealsdk::sigscan;

namespace unrealsdk::games {

BL3Hook::BL3Hook(void) {
    LOG(MISC, "Hooking as BL3");

    auto [start, size] = get_exe_range();

    auto gobjects_instr = scan(start, size, this->gobjects_sig) + 3;
    auto gobjects_ptr = read_offset<unreal::GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);
    this->gobjects = unreal::GObjects(gobjects_ptr);
}

}  // namespace unrealsdk::games
