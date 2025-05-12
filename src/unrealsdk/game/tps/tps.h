#ifndef UNREALSDK_GAME_TPS_TPS_H
#define UNREALSDK_GAME_TPS_TPS_H

#include "unrealsdk/pch.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/game/selector.h"

namespace unrealsdk::game {

class TPSHook : public BL2Hook {
   protected:
    void hexedit_array_limit_message(void) const override;

   public:
    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const override;
    [[nodiscard]] const unreal::offsets::OffsetList& get_offsets(void) const override;
};

template <>
struct GameTraits<TPSHook> {
    static constexpr auto NAME = "TPS";

    static bool matches_executable(std::string_view executable) {
        return executable == "BorderlandsPreSequel.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_TPS_TPS_H */
