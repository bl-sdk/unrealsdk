#ifndef BL2_H
#define BL2_H

#include "pch.h"

#include "games/game_hook.h"

namespace unrealsdk::games {

class BL2Hook : public GameHook {};

template <>
struct GameTraits<BL2Hook> {
    static constexpr bool IS_64BIT = false;
    static constexpr bool IS_UE4 = false;

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands2.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* BL2_H */
