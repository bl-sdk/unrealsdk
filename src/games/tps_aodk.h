#ifndef GAMES_TPS_AODK_H
#define GAMES_TPS_AODK_H

#include "pch.h"

#if defined(UE3) && defined(ARCH_X86)

#include "games/bl2.h"
#include "games/game_hook.h"

namespace unrealsdk::games {

class TPSAoDKHook : public BL2Hook {
   protected:
    void find_gnames(void) override;

   public:
    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) const override;
    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const override;
};

template <>
struct GameTraits<TPSAoDKHook> {
    static constexpr auto NAME = "TPS + AoDK";

    static bool matches_executable(const std::string& executable) {
        return executable == "BorderlandsPreSequel.exe" || executable == "TinyTina.exe";
    }
};

}  // namespace unrealsdk::games

#endif

#endif /* GAMES_TPS_AODK_H */
