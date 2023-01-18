#ifndef GAME_WL_H
#define GAME_WL_H

#include "pch.h"

#if defined(UE4) && defined(ARCH_X64)

#include "game/bl3.h"
#include "game/game_hook.h"

namespace unrealsdk::game {

class WLHook : public BL3Hook {};

template <>
struct GameTraits<WLHook> {
    static constexpr auto NAME = "Wonderlands";

    static bool matches_executable(const std::string& executable) {
        return executable == "Wonderlands.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* GAME_WL_H */
