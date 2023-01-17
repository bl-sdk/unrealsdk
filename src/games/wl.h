#ifndef GAMES_WL_H
#define GAMES_WL_H

#include "pch.h"

#if defined(UE4) && defined(ARCH_X64)

#include "games/bl3.h"
#include "games/game_hook.h"

namespace unrealsdk::games {

class WLHook : public BL3Hook {};

template <>
struct GameTraits<WLHook> {
    static constexpr auto NAME = "Wonderlands";

    static bool matches_executable(const std::string& executable) {
        return executable == "Wonderlands.exe";
    }
};

}  // namespace unrealsdk::games

#endif

#endif /* GAMES_WL_H */
