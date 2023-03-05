#ifndef GAME_TPS_TPS_H
#define GAME_TPS_TPS_H

#include "pch.h"

#if defined(UE3) && defined(ARCH_X86)

#include "game/bl2/bl2.h"
#include "game/selector.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

class TPSHook : public BL2Hook {
   protected:
    void hexedit_array_limit_message(void) const override;
    void find_gnames(void) const override;

   public:
    void fname_init(FName* name, const std::wstring& str, int32_t number) const override;
    void fname_init(FName* name, const wchar_t* str, int32_t number) const override;

    [[nodiscard]] const GNames& gnames(void) const override;
};

template <>
struct GameTraits<TPSHook> {
    static constexpr auto NAME = "TPS";

    static bool matches_executable(const std::string& executable) {
        return executable == "BorderlandsPreSequel.exe" || executable == "TinyTina.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* GAME_TPS_TPS_H */
