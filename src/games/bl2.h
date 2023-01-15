#ifndef GAMES_BL2_H
#define GAMES_BL2_H

#include "pch.h"

#include "games/game_hook.h"

namespace unrealsdk::games {

class BL2Hook : public GameHook {
   protected:
    void hook_antidebug(void) override;
    void hook_process_event(void) override;
    void hook_call_function(void) override;
    void find_gobjects(void) override;
    void find_gnames(void) override;
    void find_fname_init(void) override;
    void find_fframe_step(void) override;

    /// Pointer to FName::Init
    // Deliberately storing in a void pointer because the type changes in bl2/tps
    void* fname_init_ptr;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef void(__thiscall* fframe_step_func)(unreal::FFrame* stack,
                                               unreal::UObject* obj,
                                               void* param);
    /// Pointer to FFrame::Step
    fframe_step_func fframe_step_ptr;

   public:
    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) const override;
    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const override;
    void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const override;
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
