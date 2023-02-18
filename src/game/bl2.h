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

    /**
     * @brief Hex edits out the protection on the set command.
     */
    static void hexedit_set_command(void);

    /**
     * @brief Hex edits out the `obj dump` array limit.
     */
    static void hexedit_array_limit(void);

    /**
     * @brief Bypasses the say console command suppression.
     */
    static void hook_say_bypass(void);

    /**
     * @brief Hex edits out the `obj dump` array limit message.
     */
    virtual void hexedit_array_limit_message(void);

    void hook_process_event(void) override;
    void hook_call_function(void) override;
    void find_gobjects(void) override;
    void find_gnames(void) override;
    void find_fname_init(void) override;
    void find_fframe_step(void) override;
    void find_gmalloc(void) override;

    // Deliberately storing in a void pointer because the type changes in bl2/tps
    void* fname_init_ptr;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef void(__thiscall* fframe_step_func)(unreal::FFrame* stack,
                                               unreal::UObject* obj,
                                               void* param);
    fframe_step_func fframe_step_ptr;

    struct FMalloc;
    struct FMallocVFtable {
        void* exec;
        void*(__thiscall* malloc)(FMalloc* self, uint32_t len, uint32_t align);
        void*(__thiscall* realloc)(FMalloc* self, void* original, uint32_t len, uint32_t align);
        void*(__thiscall* free)(FMalloc* self, void* data);
    };
    struct FMalloc {
        FMallocVFtable* vftable;
    };

    FMalloc* gmalloc;

   public:
    void hook(void) override;

    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) const override;
    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const override;
    void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const override;
    [[nodiscard]] void* malloc(size_t len) const override;
    [[nodiscard]] void* realloc(void* original, size_t len) const override;
    void free(void* data) const override;
    void process_event(unreal::UObject* object,
                       unreal::UFunction* func,
                       void* params) const override;
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
