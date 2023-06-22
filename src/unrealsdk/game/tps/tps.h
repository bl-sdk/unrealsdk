#ifndef UNREALSDK_GAME_TPS_TPS_H
#define UNREALSDK_GAME_TPS_TPS_H

#include "unrealsdk/pch.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/game/selector.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

class TPSHook : public BL2Hook {
   protected:
    void hexedit_array_limit_message(void) const override;

   public:
    void fname_init(FName* name, const wchar_t* str, int32_t number) const override;
};

template <>
struct GameTraits<TPSHook> {
    static constexpr auto NAME = "TPS";

    static bool matches_executable(const std::string& executable) {
        return executable == "BorderlandsPreSequel.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_TPS_TPS_H */
