#include "pch.h"

#include "env.h"
#include "games/bl2.h"
#include "games/bl3.h"
#include "games/game_hook.h"
#include "games/tps_aodk.h"
#include "games/wl.h"
#include "sigscan.h"
#include "unrealsdk.h"

namespace unrealsdk {

// Keep an internal mutable var, but only expose a const reference
// Not the greatest fan of defining stuff outside of our namespace, but it's worth it so it's const
static std::unique_ptr<games::GameHook> game_instance;
const std::unique_ptr<games::GameHook>& game = game_instance;

}  // namespace unrealsdk

namespace unrealsdk::games {

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
constexpr void iter_hooks(const std::string& executable) {
    if constexpr (i >= std::tuple_size_v<all_known_games>) {
        throw std::runtime_error("Failed to find compatible game hook!");
    } else {
        using game = std::tuple_element_t<i, all_known_games>;
        if (GameTraits<game>::matches_executable(executable)) {
            LOG(INFO, "Using %s hook", GameTraits<game>::NAME);
            unrealsdk::game_instance = std::make_unique<game>();
            unrealsdk::game_instance->hook();
            return;
        }
        iter_hooks<i + 1>(executable);
    }
}

void hook(void) {
    auto executable = env::get(env::GAME_OVERRIDE, unrealsdk::paths.exe_path.filename().string());
    iter_hooks(executable);
}

GameHook::GameHook() {
    auto [start_val, size_val] = sigscan::get_exe_range();
    this->start = start_val;
    this->size = size_val;
}

void GameHook::hook() {
    this->hook_antidebug();
    this->hook_process_event();
    this->hook_call_function();
    this->find_gobjects();
    this->find_gnames();
    this->find_fname_init();
    this->find_fframe_step();
}

}  // namespace unrealsdk::games
