#ifndef UNREALSDK_UNREALSDK_H
#define UNREALSDK_UNREALSDK_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::game {

struct AbstractHook;

}

namespace unrealsdk::unreal {

class GNames;
class GObjects;
class UClass;
class UFunction;
struct FFrame;
struct FLazyObjectPtr;
struct FSoftObjectPtr;
struct FText;
struct TemporaryFString;

}  // namespace unrealsdk::unreal

namespace unrealsdk {

#ifndef UNREALSDK_IMPORTING

/**
 * @brief Initializes the sdk.
 * @note Logging is initialized before calling the getter, it is safe to log within it.
 *
 * @param game_getter A function which gets an instance of the hook type to use on the current game.
 * @return True if the sdk was initialized with the given game, false if it was already initialized.
 */
bool init(const std::function<std::unique_ptr<game::AbstractHook>(void)>& game_getter);

#endif

/**
 * @brief Checks if the SDK has been initialized.
 * @note Thread safe, blocks if initialization is in progress.
 * @note The console hooks are setup during initialization, but console may not immediately be
 *       available - also see `is_console_ready`.
 *
 * @return True if the SDK has been initialized.
 */
[[nodiscard]] bool is_initialized(void);

/**
 * @brief Checks if the sdk's console hook is ready to output text.
 *
 * @return True if the console hook is ready, false otherwise.
 */
[[nodiscard]] bool is_console_ready(void);

// ================ Remaining functions have undefined behaviour if not initialized ================

/**
 * @brief Gets a reference to the GObjects wrapper.
 *
 * @return A reference to the GObjects wrapper.
 */
[[nodiscard]] const unreal::GObjects& gobjects(void);

/**
 * @brief Gets a reference to the GNames wrapper.
 *
 * @return A reference to the GNames wrapper.
 */
[[nodiscard]] const unreal::GNames& gnames(void);

/**
 * @brief Calls unreal's malloc function.
 *
 * @tparam T The type to cast the return to.
 * @param len The amount of bytes to allocate.
 * @return A pointer to the allocated memory.
 */
[[nodiscard]] void* u_malloc(size_t len);
template <typename T>
[[nodiscard]] T* u_malloc(size_t len) {
    return reinterpret_cast<T*>(u_malloc(len));
}

/**
 * @brief Calls unreal's realloc function.
 *
 * @tparam T The type to cast the return to.
 * @param original The original memory to re-allocate.
 * @param len The amount of bytes to allocate.
 * @return A pointer to the re-allocated memory.
 */
[[nodiscard]] void* u_realloc(void* original, size_t len);
template <typename T>
[[nodiscard]] T* u_realloc(void* original, size_t len) {
    return reinterpret_cast<T*>(u_realloc(original, len));
}

/**
 * @brief Calls unreal's free function.
 *
 * @param data The memory to free.
 */
void u_free(void* data);

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
[[nodiscard]] unreal::UObject* construct_object(unreal::UClass* cls,
                                                unreal::UObject* outer,
                                                const unreal::FName& name = {0, 0},
                                                decltype(unreal::UObject::ObjectFlags) flags = 0,
                                                unreal::UObject* template_obj = nullptr);

/**
 * @brief Loads a package, and all it's contained objects.
 * @note This function may block for several seconds while the package is loaded.
 *
 * @param name The package's name.
 * @param flags The loading flags to use.
 * @return The loaded `Package` object.
 */
// NOLINTNEXTLINE(modernize-use-nodiscard)
unreal::UObject* load_package(std::wstring_view name, uint32_t flags = 0);

/**
 * @brief Finds an object by name.
 *
 * @param cls The object's class (or it's name).
 * @param name The object's full path name.
 * @return The object, or nullptr if unable to find.
 */
[[nodiscard]] unreal::UObject* find_object(unreal::UClass* cls, std::wstring_view name);
[[nodiscard]] unreal::UObject* find_object(const unreal::FName& cls, std::wstring_view name);
[[nodiscard]] unreal::UObject* find_object(std::wstring_view cls, std::wstring_view name);

// Everything in this namespace is used by sdk internals, and is generally not useful in user code.
// For example, `fname_init` is called by the `FName` constructor, so there's no real reason to call
// it over just constructing one directly.
namespace internal {

/**
 * @brief Calls `FName::Init`, set to add new names and split numbers.
 *
 * @param name Pointer to the name to initialize.
 * @param str The string to initialize the name to. Must be null terminated.
 * @param number The number to initialize the name to.
 */
void fname_init(unreal::FName* name, const wchar_t* str, int32_t number);
void fname_init(unreal::FName* name, const std::wstring& str, int32_t number);

/**
 * @brief Calls `FFrame::Step`.
 *
 * @param frame The frame to step.
 * @param obj The object the frame is coming from.
 * @param param The parameter.
 */
void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param);

/**
 * @brief Calls `UObject::ProcessEvent`.
 *
 * @param object The object to process an event on.
 * @param func The function to call.
 * @param params The function's params
 */
void process_event(unreal::UObject* object, unreal::UFunction* func, void* params);

/**
 * @brief Calls `UConsole::OutputText` to write to the UE console.
 * @note Calls to this before the console hook is ready are silently dropped.
 *
 * @param str The string to write.
 */
void uconsole_output_text(std::wstring_view str);

/**
 * @brief Calls `UObject::PathName` on the given object.
 *
 * @param obj The object to get the name of.
 * @return The object's name
 */
[[nodiscard]] std::wstring uobject_path_name(const unreal::UObject* obj);

/**
 * @brief Calls `FText::AsCultureInvariant`.
 *
 * @param name Pointer to the text to initialize.
 * @param str The string to initialize the text to.
 */
void ftext_as_culture_invariant(unreal::FText* text, unreal::TemporaryFString&& str);

/**
 * @brief Assigns an object to a `FSoftObjectPtr` or `FLazyObjectPtr`.
 *
 * @param ptr Pointer to the pointer to assign.
 * @param obj The object to assign.
 */
void fsoftobjectptr_assign(unreal::FSoftObjectPtr* ptr, const unreal::UObject* obj);
void flazyobjectptr_assign(unreal::FLazyObjectPtr* ptr, const unreal::UObject* obj);

}  // namespace internal

}  // namespace unrealsdk

#endif /* UNREALSDK_UNREALSDK_H */
