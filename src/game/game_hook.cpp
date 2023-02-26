#include "pch.h"

#include "env.h"
#include "game/bl2.h"
#include "game/bl3.h"
#include "game/game_hook.h"
#include "game/tps_aodk.h"
#include "game/wl.h"
#include "memory.h"
#include "unrealsdk.h"

namespace unrealsdk::game {

static std::unique_ptr<GameHook> hook_instance;

#pragma region Wrappers

const unreal::GObjects& gobjects(void) {
    return hook_instance->gobjects;
}
const unreal::GNames& gnames(void) {
    return hook_instance->gnames;
}
void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) {
    hook_instance->fname_init(name, str, number);
}
void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) {
    hook_instance->fname_init(name, str, number);
}
void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) {
    hook_instance->fframe_step(frame, obj, param);
}
void* malloc(size_t len) {
    return hook_instance->malloc(len);
}
void* realloc(void* original, size_t len) {
    return hook_instance->realloc(original, len);
}
void free(void* data) {
    return hook_instance->free(data);
}
void process_event(unreal::UObject* object, unreal::UFunction* function, void* params) {
    hook_instance->process_event(object, function, params);
}
unreal::UObject* construct_object(unreal::UClass* cls,
                                  unreal::UObject* outer,
                                  const unreal::FName& name,
                                  decltype(unreal::UObject::ObjectFlags) flags,
                                  unreal::UObject* template_obj) {
    return hook_instance->construct_object(cls, outer, name, flags, template_obj);
}

#pragma endregion

#pragma region Hook Selection

// Tuple of all hook types to consider.
// The first matching hook will be used, order matters.
#ifdef ARCH_X64
#ifdef UE4
using all_known_games = std::tuple<BL3Hook, WLHook>;
#else
#error No known games for UE3 x64
#endif
#else
#ifdef UE4
#error No known games for UE4 x86
#else
using all_known_games = std::tuple<BL2Hook, TPSAoDKHook>;
#endif
#endif

/**
 * @brief Recursive helper function to find the right game hook.
 *
 * @tparam i Index of the game class being tested this iteration. Picked up automatically.
 * @param executable The executable name to match against.
 */
template <int i = 0>
constexpr void find_correct_hook(const std::string& executable) {
    if constexpr (i >= std::tuple_size_v<all_known_games>) {
        throw std::runtime_error("Failed to find compatible game hook!");
    } else {
        using game = std::tuple_element_t<i, all_known_games>;
        if (GameTraits<game>::matches_executable(executable)) {
            LOG(INFO, "Using %s hook", GameTraits<game>::NAME);
            hook_instance = std::make_unique<game>();
            hook_instance->hook();
            return;
        }
        find_correct_hook<i + 1>(executable);
    }
}

void init(void) {
    auto executable = env::get(env::GAME_OVERRIDE, unrealsdk::paths.exe_path.filename().string());
    find_correct_hook(executable);
}

#pragma endregion

#pragma region Hook Classes

void GameHook::hook() {
    this->hook_process_event();
    this->hook_call_function();
    this->find_gobjects();
    this->find_gnames();
    this->find_fname_init();
    this->find_fframe_step();
    this->find_gmalloc();
    this->find_construct_object();
}

uint32_t GameHook::get_alignment(size_t len) {
    static auto override = env::get_numeric<uint32_t>(env::ALLOC_ALIGNMENT);
    if (override != 0) {
        return override;
    }

    // Not all UE versions support alignment 0 = auto, so reimplement it ourselves
    static const constexpr auto LARGE_ALIGNMENT_THRESHOLD = 16;
    static const constexpr auto LARGE_ALIGNMENT = 16;
    static const constexpr auto SMALL_ALIGNMENT = 8;

    return len >= LARGE_ALIGNMENT_THRESHOLD ? LARGE_ALIGNMENT : SMALL_ALIGNMENT;
}

#pragma endregion

}  // namespace unrealsdk::game
