#ifndef GAMES_TPS_AODK_H
#define GAMES_TPS_AODK_H

#include "pch.h"

#include "games/bl2.h"
#include "games/game_hook.h"

namespace unrealsdk::games {

class TPSAoDKHook : public BL2Hook {
   protected:
    void find_gnames(void) override;

   public:
    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) override;
};

template <>
struct GameTraits<TPSAoDKHook> {
    static constexpr auto NAME = "TPS + AoDK";
    static constexpr bool IS_64BIT = false;
    static constexpr bool IS_UE4 = false;

    static bool matches_executable(const std::string& executable) {
        return executable == "BorderlandsPreSequel.exe" || executable == "TinyTina.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* GAMES_TPS_AODK_H */
