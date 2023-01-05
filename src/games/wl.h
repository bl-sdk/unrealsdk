#ifndef GAMES_WL_H
#define GAMES_WL_H

#include "pch.h"

#include "games/bl3.h"
#include "games/game_hook.h"

namespace unrealsdk::games {

class WLHook : public BL3Hook {};

template <>
struct GameTraits<WLHook> {
    static constexpr auto NAME = "Wonderlands";
    static constexpr bool IS_64BIT = true;
    static constexpr bool IS_UE4 = true;

    static bool matches_executable(const std::string& executable) {
        return executable == "Wonderlands.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* GAMES_WL_H */
