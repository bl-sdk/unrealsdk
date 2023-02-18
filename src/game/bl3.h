#ifndef GAME_BL3_H
#define GAME_BL3_H

#include "pch.h"

#if defined(UE4) && defined(ARCH_X64)

#include "game/game_hook.h"
#include "unreal/structs/fname.h"

namespace unrealsdk::game {

class BL3Hook : public GameHook {
   protected:
    void hook_process_event(void) override;
    void hook_call_function(void) override;
    void find_gobjects(void) override;
    void find_gnames(void) override;
    void find_fname_init(void) override;
    void find_fframe_step(void) override;
    void find_gmalloc(void) override;

    using fname_init_func = unreal::FName(const wchar_t* str, int32_t number, int32_t find_type);
    fname_init_func* fname_init_ptr;

    using fframe_step_func = void(unreal::FFrame* stack, unreal::UObject* obj, void* param);
    fframe_step_func* fframe_step_ptr;

    using fmemory_malloc_func = void*(uint64_t len, uint32_t align);
    using fmemory_realloc_func = void*(void* original, uint64_t len, uint32_t align);
    using fmemory_free_func = void(void* data);
    fmemory_malloc_func* fmemory_malloc_ptr;
    fmemory_realloc_func* fmemory_realloc_ptr;
    fmemory_free_func* fmemory_free_ptr;

   public:
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
struct GameTraits<BL3Hook> {
    static constexpr auto NAME = "Borderlands 3";

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands3.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* GAME_BL3_H */
