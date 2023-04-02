#include "pch.h"

#include "env.h"
#include "game/abstract_hook.h"
#include "game/bl2/bl2.h"
#include "game/bl3/bl3.h"
#include "game/tps/tps.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

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
template <int i = 0>
static std::unique_ptr<AbstractHook> find_correct_hook(const std::string& executable) {
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

std::unique_ptr<AbstractHook> select_based_on_executable(void) {
    std::filesystem::path exe_path{};

    std::array<char, FILENAME_MAX> buf{};
    if (GetModuleFileNameA(nullptr, buf.data(), (DWORD)buf.size()) > 0) {
        exe_path = std::filesystem::path(buf.data());
    } else {
        LOG(ERROR, "Failed to get main executable's path!");
    }

    auto executable = env::get(env::GAME_OVERRIDE, exe_path.filename().string());
    return find_correct_hook(executable);
}

#pragma endregion

}  // namespace unrealsdk::game
