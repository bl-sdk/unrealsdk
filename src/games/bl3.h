#ifndef GAMES_BL3_H
#define GAMES_BL3_H

#include "pch.h"

#include "games/game_hook.h"
#include "unreal/structs/fname.h"

namespace unrealsdk::games {

class BL3Hook : public GameHook {
   protected:
    void hook_process_event(void) override;
    void hook_call_function(void) override;
    void find_gobjects(void) override;
    void find_gnames(void) override;
    void find_fname_init(void) override;
    void find_fframe_step(void) override;

    using fname_init_func = unreal::FName(const wchar_t* str, int32_t number, int32_t find_type);
    /// Pointer to FName::Init
    fname_init_func* fname_init_ptr;

    using fframe_step_func = void(unreal::FFrame* stack, unreal::UObject* obj, void* param);
    /// Pointer to FFrame::Step
    fframe_step_func* fframe_step_ptr;

   public:
    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) const override;
    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const override;
    void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const override;
};

template <>
struct GameTraits<BL3Hook> {
    static constexpr auto NAME = "Borderlands 3";
    static constexpr bool IS_64BIT = true;
    static constexpr bool IS_UE4 = true;

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands3.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* GAMES_BL3_H */
