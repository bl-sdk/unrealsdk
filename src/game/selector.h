#ifndef GAME_SELECTOR_H
#define GAME_SELECTOR_H

#include "pch.h"

#include "game/abstract_hook.h"

namespace unrealsdk::game {

/**
 * @brief Selects a game hook based on the current executable's name.
 *
 * @return A pointer to the game hook instance.
 */
std::unique_ptr<AbstractHook> select_based_on_executable(void);

/**
 * @brief Traits class describing a game hook.
 *
 * @tparam T The Hook class that's being described.
 */
template <typename T, typename = std::enable_if_t<std::is_base_of_v<AbstractHook, T>, void>>
struct GameTraits {
    static constexpr const char* const NAME{};

    /**
     * @brief Checks if this hook matches the given executable name.
     *
     * @param executable The executable name to match.
     * @return True if the hook matches it.
     */
    static bool matches_executable(const std::string& executable);
};

}  // namespace unrealsdk::game

#endif /* GAME_SELECTOR_H */
