#ifndef UNREALSDK_UNREALSDK_H
#define UNREALSDK_UNREALSDK_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::game {

struct AbstractHook;

}

using namespace unrealsdk::unreal;
namespace unrealsdk::unreal {

class GNames;
class GObjects;
class UClass;
class UFunction;
struct FFrame;

}  // namespace unrealsdk::unreal

namespace unrealsdk {

#ifndef UNREALSDK_IMPORTING
/**
 * @brief Initializes the sdk.
 *
 * @param game An instance of the hook type to use to hook the current game.
 * @return True if the sdk was initialized with the given game, false if it was already initialized.
 */
bool init(std::unique_ptr<game::AbstractHook>&& game);
#endif

/**
 * @brief Checks if the SDK has been initialized.
 * @note Thread safe, blocks if initialization is in progress.
 *
 * @return True if the SDK has been initialized.
 */
UNREALSDK_CAPI bool is_initialized(void) UNREALSDK_CAPI_SUFFIX;

// ================ Remaining functions have undefined behaviour if not initialized ================

/**
 * @brief Gets a reference to the GObjects wrapper.
 *
 * @return A reference to the GObjects wrapper.
 */
[[nodiscard]] const GObjects& gobjects(void);

/**
 * @brief Gets a reference to the GNames wrapper.
 *
 * @return A reference to the GNames wrapper.
 */
[[nodiscard]] const GNames& gnames(void);

/**
 * @brief Calls FName::Init, set to add new names and split numbers.
 *
 * @param name Pointer to the name to initialize.
 * @param str The string to initialize the name to.
 * @param number The number to initialize the name to.
 */
UNREALSDK_CAPI void fname_init(FName* name,
                               const wchar_t* str,
                               int32_t number) UNREALSDK_CAPI_SUFFIX;
void fname_init(FName* name, const std::wstring& str, int32_t number);

/**
 * @brief Calls FFrame::Step.
 *
 * @param frame The frame to step.
 * @param obj The object the frame is coming from.
 * @param param The parameter.
 */
UNREALSDK_CAPI void fframe_step(FFrame* frame, UObject* obj, void* param) UNREALSDK_CAPI_SUFFIX;

/**
 * @brief Calls unreal's malloc function.
 *
 * @tparam T The type to cast the return to.
 * @param len The amount of bytes to allocate.
 * @return A pointer to the allocated memory.
 */
UNREALSDK_CAPI [[nodiscard]] void* u_malloc(size_t len) UNREALSDK_CAPI_SUFFIX;
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
UNREALSDK_CAPI [[nodiscard]] void* u_realloc(void* original, size_t len) UNREALSDK_CAPI_SUFFIX;
template <typename T>
[[nodiscard]] T* u_realloc(void* original, size_t len) {
    return reinterpret_cast<T*>(u_realloc(original, len));
}

/**
 * @brief Calls unreal's free function.
 *
 * @param data The memory to free.
 */
UNREALSDK_CAPI void u_free(void* data) UNREALSDK_CAPI_SUFFIX;

/**
 * @brief Calls `UObject::ProcessEvent`.
 *
 * @param object The object to process an event on.
 * @param func The function to call.
 * @param params The function's params
 */
UNREALSDK_CAPI void process_event(UObject* object, UFunction* func, void* params);

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
 *
 * @param str The string to write.
 */
void uconsole_output_text(const std::wstring& str);

/**
 * @brief Calls `UObject::PathName` on the given object.
 *
 * @param obj The object to get the name of.
 * @return The object's name
 */
[[nodiscard]] std::wstring uobject_path_name(const UObject* obj);

/**
 * @brief Finds an object by name.
 *
 * @param cls The object's class (or it's name).
 * @param name The object's full path name.
 * @return The object, or nullptr if unable to find.
 */
[[nodiscard]] UObject* find_object(UClass* cls, const std::wstring& name);
[[nodiscard]] UObject* find_object(const FName& cls, const std::wstring& name);
[[nodiscard]] UObject* find_object(const std::wstring& cls, const std::wstring& name);

}  // namespace unrealsdk

#endif /* UNREALSDK_UNREALSDK_H */
