#ifndef GAME_BL2_H
#define GAME_BL2_H

#include "pch.h"

#if defined(UE3) && defined(ARCH_X86)

#include "game/game_hook.h"

namespace unrealsdk::game {

class BL2Hook : public GameHook {
   protected:
    /**
     * @brief Hooks the antidebug functions and disables them.
     */
    static void hook_antidebug(void);

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
    void hook(void) override;

    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) const override;
    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const override;
    void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const override;
};

template <>
struct GameTraits<BL2Hook> {
    static constexpr auto NAME = "Borderlands 2";

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands2.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* GAME_BL2_H */
