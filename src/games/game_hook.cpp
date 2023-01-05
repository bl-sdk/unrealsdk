#include "pch.h"

#include "env.h"
#include "games/bl2.h"
#include "games/bl3.h"
#include "games/game_hook.h"
#include "games/tps_aodk.h"
#include "games/wl.h"
#include "unrealsdk.h"

namespace unrealsdk {

// Keep an internal mutable var, but only expose a const reference
// Not the greatest fan of defining stuff outside of our namespace, but it's worth it so it's const
static std::unique_ptr<games::GameHook> game_instance;
const std::unique_ptr<games::GameHook>& game = game_instance;

}  // namespace unrealsdk

namespace unrealsdk::games {

/**
 * @brief Tuple of all hook types to consider.
 * @note The first matching hook will be used, order matters.
 */
// TODO: generic hooks
using all_known_games = std::tuple<BL2Hook, TPSAoDKHook, BL3Hook, WLHook>;

#ifdef ARCH_X64
static constexpr auto WANT_64BIT = true;
#else
constexpr auto WANT_64BIT = false;
#endif
#ifdef UE4
static constexpr auto WANT_UE4 = true;
#else
constexpr auto WANT_UE4 = false;
#endif

/**
 * @brief Recursive helper function to find the right game hook, which discards games with the wrong
 *        architecture/ue version at compile time.
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
        if constexpr (GameTraits<game>::IS_64BIT == WANT_64BIT
                      && GameTraits<game>::IS_UE4 == WANT_UE4) {
            if (GameTraits<game>::matches_executable(executable)) {
                LOG(INFO, "Using %s hook", GameTraits<game>::NAME);
                unrealsdk::game_instance = std::make_unique<game>();
                return;
            }
        }
        iter_hooks<i + 1>(executable);
    }
}

void hook(void) {
    auto executable = env::get(env::GAME_OVERRIDE, unrealsdk::paths.exe_path.filename().string());
    iter_hooks(executable);
}

}  // namespace unrealsdk::games
