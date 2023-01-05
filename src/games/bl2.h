#ifndef GAMES_BL2_H
#define GAMES_BL2_H

#include "pch.h"

#include "games/game_hook.h"
#include "sigscan.h"

using unrealsdk::sigscan::Pattern;

namespace unrealsdk::games {

class BL2Hook : public GameHook {
    const Pattern gobjects_sig{"\x00\x00\x00\x00\x8B\x04\xB1\x8B\x40\x08",
                               "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF", 0};
    const Pattern gnames_sig{"\x00\x00\x00\x00\x83\x3C\x81\x00\x74\x5C",
                             "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF", 0};

   public:
    BL2Hook(void);
};

template <>
struct GameTraits<BL2Hook> {
    static constexpr bool IS_64BIT = false;
    static constexpr bool IS_UE4 = false;

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands2.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* GAMES_BL2_H */
