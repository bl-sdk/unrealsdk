#ifndef UNREALSDK_GAME_AODK_AODK_H
#define UNREALSDK_GAME_AODK_AODK_H

#include "unrealsdk/pch.h"

#if defined(UE3) && defined(ARCH_X86)

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/game/selector.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

class AoDKHook : public BL2Hook {
   protected:
    void find_gnames(void) const override;

   public:
    [[nodiscard]] const GNames& gnames(void) const override;
};

template <>
struct GameTraits<AoDKHook> {
    static constexpr auto NAME = "AoDK";

    static bool matches_executable(const std::string& executable) {
        return executable == "TinyTina.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_AODK_AODK_H */
