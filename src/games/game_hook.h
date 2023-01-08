#ifndef GAMES_GAME_HOOK_H
#define GAMES_GAME_HOOK_H

#include "pch.h"

#include "unreal/wrappers/gnames.h"
#include "unreal/wrappers/gobjects.h"

namespace unrealsdk::unreal {

struct FName;

}

namespace unrealsdk::games {

/**
 * @brief Base class for hooking a game.
 */
struct GameHook {
   protected:
    /// Start address of the executable, to be used in sigscans
    uintptr_t start;
    /// Size of the executable, to be used in sigscans
    size_t size;

    /**
     * @brief Applies any anti-debug hooks, as required.
     */
    virtual void hook_antidebug(void) {}

    /**
     * @brief Hooks `ProcessEvent`, points it at the hook manager, and sets up such that
     *        `process_event` may be called.
     */
    virtual void hook_process_event(void) = 0;

    /**
     * @brief Finds GObjects, and populates the wrapper member.
     */
    virtual void find_gobjects(void) = 0;

    /**
     * @brief Finds GNames, and populates the wrapper member.
     */
    virtual void find_gnames(void) = 0;

    /**
     * @brief Finds `FName::Init`, and sets up such that `fname_init` may be called.
     */
    virtual void find_fname_init(void) = 0;

   public:
    GameHook(void);
    virtual ~GameHook() = default;

    /// Wrapper around gobjects
    unreal::GObjects gobjects;
    /// Wrapper around gnames
    unreal::GNames gnames;

    /**
     * @brief Hooks the current game.
     * @note May apply game-specific hooks or hex edits as needed.
     */
    virtual void hook(void);

    /**
     * @brief Call FName::Init, set to add new names and split numbers.
     *
     * @param name Pointer to the name to initalize.
     * @param str The string to initalize the name to.
     * @param number The number to initalize the name to.
     */
    virtual void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) = 0;
    virtual void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) = 0;
};

/**
 * @brief Traits class describing a game hook.
 *
 * @tparam T The Hook class that's being described.
 */
template <typename T, typename = std::enable_if_t<std::is_base_of_v<GameHook, T>, void>>
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

#endif /* GAMES_GAME_HOOK_H */
