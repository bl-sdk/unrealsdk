#ifndef GAMES_BL2_H
#define GAMES_BL2_H

#include "pch.h"

#include "games/game_hook.h"

namespace unrealsdk::games {

class BL2Hook : public GameHook {
   protected:
    void hook_antidebug(void) override;
    void find_gobjects(void) override;
    void find_gnames(void) override;
    void find_fname_init(void) override;

    /// Pointer to FName::Init function
    void* fname_init_ptr;

   public:
    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) override;
    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) override;
};

template <>
struct GameTraits<BL2Hook> {
    static constexpr auto NAME = "Borderlands 2";
    static constexpr bool IS_64BIT = false;
    static constexpr bool IS_UE4 = false;

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands2.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* GAMES_BL2_H */
