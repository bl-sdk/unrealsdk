#include "unrealsdk/pch.h"

#include "unrealsdk/config.h"
#include "unrealsdk/game/abstract_hook.h"
#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/game/tps/tps.h"
#include "unrealsdk/utils.h"

#ifndef UNREALSDK_IMPORTING

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

// Tuple of all hook types to consider.
// The first matching hook will be used, order matters.
#ifdef ARCH_X64
#ifdef UE4
using all_known_games = std::tuple<BL3Hook>;
#else
#error No known games for UE3 x64
#endif
#else
#ifdef UE4
#error No known games for UE4 x86
#else
using all_known_games = std::tuple<BL2Hook, TPSHook>;
#endif
#endif

/**
 * @brief Recursive helper function to find the right game hook.
 *
 * @tparam i Index of the game class being tested this iteration. Picked up automatically.
 * @param executable The executable name to match against.
 */
template <size_t i = 0>
std::unique_ptr<AbstractHook> find_correct_hook(std::string_view executable) {
    if constexpr (i >= std::tuple_size_v<all_known_games>) {
        throw std::runtime_error("Failed to find compatible game hook!");
    } else {
        using game = std::tuple_element_t<i, all_known_games>;
        if (GameTraits<game>::matches_executable(executable)) {
            LOG(INFO, "Using {} hook", GameTraits<game>::NAME);
            return std::make_unique<game>();
        }
        return find_correct_hook<i + 1>(executable);
    }
}

}  // namespace

std::unique_ptr<AbstractHook> select_based_on_executable(void) {
    auto executable_filename = utils::get_executable().filename().string();
    return find_correct_hook(config::get_str("exe_override").value_or(executable_filename));
}

}  // namespace unrealsdk::game

#endif
