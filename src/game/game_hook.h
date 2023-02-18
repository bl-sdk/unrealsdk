#ifndef GAME_GAME_HOOK_H
#define GAME_GAME_HOOK_H

#include "pch.h"

#include "unreal/structs/fframe.h"
#include "unreal/wrappers/gnames.h"
#include "unreal/wrappers/gobjects.h"

namespace unrealsdk::unreal {

struct FName;
class UObject;
class UFunction;

}  // namespace unrealsdk::unreal

namespace unrealsdk::game {

/**
 * @brief Hooks the current game, and initalizes all other functions in this module.
 */
void init(void);

/**
 * @brief Gets a reference to the GObjects wrapper.
 *
 * @return A reference to the GObjects wrapper.
 */
const unreal::GObjects& gobjects(void);

/**
 * @brief Gets a reference to the GNames wrapper.
 *
 * @return A reference to the GNames wrapper.
 */
const unreal::GNames& gnames(void);

/**
 * @brief Calls FName::Init, set to add new names and split numbers.
 *
 * @param name Pointer to the name to initalize.
 * @param str The string to initalize the name to.
 * @param number The number to initalize the name to.
 */
void fname_init(unreal::FName* name, const std::wstring& str, int32_t number);
void fname_init(unreal::FName* name, const wchar_t* str, int32_t number);

/**
 * @brief Calls FFrame::Step.
 *
 * @param frame The frame to step.
 * @param obj The object the frame is coming from.
 * @param param The parameter.
 */
void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param);

/**
 * @brief Calls unreal's malloc function.
 *
 * @tparam T The type to cast the return to.
 * @param len The amount of bytes to allocate.
 * @return A pointer to the allocated memory.
 */
[[nodiscard]] void* malloc(size_t len);
template <typename T>
[[nodiscard]] T* malloc(size_t len) {
    return reinterpret_cast<T*>(malloc(len));
}

/**
 * @brief Calls unreal's realloc function.
 *
 * @tparam T The type to cast the return to.
 * @param original The original memory to re-allocate.
 * @param len The amount of bytes to allocate.
 * @return A pointer to the re-allocated memory.
 */
[[nodiscard]] void* realloc(void* original, size_t len);
template <typename T>
[[nodiscard]] T* realloc(void* original, size_t len) {
    return reinterpret_cast<T*>(realloc(original, len));
}

/**
 * @brief Calls unreal's free function.
 *
 * @param data The memory to free.
 */
void free(void* data);

/**
 * @brief Calls `UObject::ProcessEvent`.
 *
 * @param object The object to process an event on.
 * @param func The function to call.
 * @param params The function's params
 */
void process_event(unreal::UObject* object, unreal::UFunction* func, void* params);

#pragma region Hook Classes

/**
 * @brief Base class for hooking a game.
 */
struct GameHook {
   protected:
#pragma region Virtual Functions

    /**
     * @brief Hooks `UObject::ProcessEvent` and points it at the hook manager.
     */
    virtual void hook_process_event(void) = 0;

    /**
     * @brief Hooks `UObject::CallFunction` and points it at the hook manager.
     */
    virtual void hook_call_function(void) = 0;

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

    /**
     * @brief Finds `FFrame::Step`, and sets up such that `fframe_step` may be called.
     */
    virtual void find_fframe_step(void) = 0;

    /**
     * @brief Finds `GMalloc`, and sets up such that `malloc`, `realloc`, and `free` may be called.
     */
    virtual void find_gmalloc(void) = 0;

#pragma endregion

    /**
     * @brief Helper to get the alignment for use with the GMalloc functions.
     *
     * @param len The amount of bytes requested to be allocated
     * @return The alignment.
     */
    static uint32_t get_alignment(size_t len);

   public:
    GameHook(void) = default;
    virtual ~GameHook() = default;

    /**
     * @brief Hooks the current game.
     * @note May be overridden to apply game-specific hooks or hex edits as needed.
     */
    virtual void hook(void);

    // Inner objects/methods accessed by the globals function wrappers
    unreal::GObjects gobjects;
    unreal::GNames gnames;
    virtual void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) const = 0;
    virtual void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const = 0;
    virtual void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const = 0;
    [[nodiscard]] virtual void* malloc(size_t len) const = 0;
    [[nodiscard]] virtual void* realloc(void* original, size_t len) const = 0;
    virtual void free(void* data) const = 0;
    virtual void process_event(unreal::UObject* object,
                               unreal::UFunction* func,
                               void* params) const = 0;
};

/**
 * @brief Traits class describing a game hook.
 *
 * @tparam T The Hook class that's being described.
 */
template <typename T, typename = std::enable_if_t<std::is_base_of_v<GameHook, T>, void>>
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

#pragma endregion

}  // namespace unrealsdk::game

#endif /* GAME_GAME_HOOK_H */
