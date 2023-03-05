#ifndef GAME_GAME_HOOK_H
#define GAME_GAME_HOOK_H

#include "pch.h"

#include "unreal/classes/uobject.h"
#include "unreal/structs/fname.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::unreal {

class GNames;
class GObjects;
class UClass;
class UFunction;
struct FFrame;

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
const GObjects& gobjects(void);

/**
 * @brief Gets a reference to the GNames wrapper.
 *
 * @return A reference to the GNames wrapper.
 */
const GNames& gnames(void);

/**
 * @brief Calls FName::Init, set to add new names and split numbers.
 *
 * @param name Pointer to the name to initalize.
 * @param str The string to initalize the name to.
 * @param number The number to initalize the name to.
 */
void fname_init(FName* name, const std::wstring& str, int32_t number);
void fname_init(FName* name, const wchar_t* str, int32_t number);

/**
 * @brief Calls FFrame::Step.
 *
 * @param frame The frame to step.
 * @param obj The object the frame is coming from.
 * @param param The parameter.
 */
void fframe_step(FFrame* frame, UObject* obj, void* param);

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
void process_event(UObject* object, UFunction* func, void* params);

/**
 * @brief Constructs a new object
 *
 * @param cls The class to construct. Required.
 * @param outer The outer object to construct the new object under. Required.
 * @param name The new object's name.
 * @param flags Object flags to set.
 * @param template_obj The template object to use.
 * @return The constructed object.
 */
[[nodiscard]] UObject* construct_object(UClass* cls,
                                        UObject* outer,
                                        const FName& name = {0, 0},
                                        decltype(UObject::ObjectFlags) flags = 0,
                                        UObject* template_obj = nullptr);

/**
 * @brief Calls `UConsole::OutputText` to write to the UE console.
 */
void uconsole_output_text(const std::wstring& str);

#pragma region Hook Classes

/**
 * @brief Base class for hooking a game.
 */
struct GameHook {
   protected:
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
     * @brief Hooks the current game and sets up all other functions on this object.
     */
    virtual void hook(void) = 0;

    // Inner methods accessed by the global wrappers
    [[nodiscard]] virtual const GObjects& gobjects(void) const = 0;
    [[nodiscard]] virtual const GNames& gnames(void) const = 0;
    virtual void fname_init(FName* name, const std::wstring& str, int32_t number) const = 0;
    virtual void fname_init(FName* name, const wchar_t* str, int32_t number) const = 0;
    virtual void fframe_step(FFrame* frame, UObject* obj, void* param) const = 0;
    [[nodiscard]] virtual void* malloc(size_t len) const = 0;
    [[nodiscard]] virtual void* realloc(void* original, size_t len) const = 0;
    virtual void free(void* data) const = 0;
    virtual void process_event(UObject* object, UFunction* func, void* params) const = 0;
    [[nodiscard]] virtual UObject* construct_object(UClass* cls,
                                                    UObject* outer,
                                                    const FName& name,
                                                    decltype(UObject::ObjectFlags) flags,
                                                    UObject* template_obj) const = 0;
    virtual void uconsole_output_text(const std::wstring& str) const = 0;
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
