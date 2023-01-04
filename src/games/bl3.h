#ifndef BL3_H
#define BL3_H

#include "pch.h"

#include "games/game_hook.h"

namespace unrealsdk::games {

class BL3Hook : public GameHook {};

template <>
struct GameTraits<BL3Hook> {
    static constexpr bool IS_64BIT = true;
    static constexpr bool IS_UE4 = true;

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands3.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* BL3_H */
