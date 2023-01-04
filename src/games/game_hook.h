#ifndef GAME_HOOK_H
#define GAME_HOOK_H

#include "pch.h"

namespace unrealsdk::games {

/**
 * @brief Base class for hooking a game.
 */
class GameHook {
   protected:
    /**
     * @brief Constructs a new Game Hook, and applies any default hexedits or hooks.
     */
    GameHook() = default;

   public:
    virtual ~GameHook() = default;
};

/**
 * @brief Traits class describing a game hook.
 *
 * @tparam T The Hook class that's being described.
 */
template<typename T, typename = std::enable_if_t<std::is_base_of_v<GameHook, T>, void>>
struct GameTraits {
    /// True if this hook is for a 64-bit game
    static constexpr bool IS_64BIT{};
    /// True if this hook is for a UE4 game
    static constexpr bool IS_UE4{};

    /**
     * @brief Checks if this hook matches the given executable name.
     *
     * @param executable The executable name to match.
     * @return True if the hook matches it.
     */
    static bool matches_executable(const std::string& executable);
};

/**
 * @brief Hooks the current game.
 * @note Initalizes `unrealsdk::game`.
 */
void hook(void);

}  // namespace unrealsdk::games

#endif /* GAME_HOOK_H */
