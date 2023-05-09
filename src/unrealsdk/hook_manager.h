#ifndef UNREALSDK_HOOK_MANAGER_H
#define UNREALSDK_HOOK_MANAGER_H

#include "unrealsdk/pch.h"
#include <string>
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/property_proxy.h"

namespace unrealsdk::unreal {

class UObject;
class UFunction;

class WrappedStruct;

}  // namespace unrealsdk::unreal

namespace unrealsdk::hook_manager {

/// What type of hook to add - i.e. when the callback runs
/// Note that the order callbacks within the same type (on the same function) are run is undefined.
enum class Type : uint8_t {
    PRE,                 /// Before running the hooked function.
    POST,                /// After the hooked function, only if it was allowed to run.
    POST_UNCONDITIONAL,  /// After the hooked function, even if it got blocked.
};

/// Information about a hooked function call
struct Details {
    /// The object the hooked function was called on.
    unreal::UObject* obj{};

    /// The arguments the hooked function was called with. While this is mutable, modifying it will
    /// *not* modify the actual function arguments.
    unreal::WrappedStruct* args{};

    /// A proxy for the return value. During pre-hooks, it's an unset value, and setting it will
    /// overwrite the return value of the function call. Whatever value is set when pre-hook
    /// processing is complete will be used, if multiple hooks all want to overwrite it they will
    /// need to cooperate.
    /// During post-hooks, it's set to the return value of the unreal function. This may have been
    /// overwritten by a pre-hook. If execution was blocked and it was not overwritten during a
    /// pre-hook, it will still be an unset value. Note that while there may be a `ReturnValue`
    /// property in the args struct, it is not necessarily correct, this always will be.
    unreal::PropertyProxy ret{nullptr};

    /// The function which was called, bound to the same object. Can be used to re-call it.
    unreal::BoundFunction func{};
};

/// The signature of hook callbacks.
/// For pre-hooks, returns if to block execution - if any pre-hook returns true, the unreal function
/// will not be run.
/// In post-hooks the return value is ignored.
using Callback = bool(Details&);

/**
 * @brief Toggles logging all unreal function calls. Best used in short bursts for debugging.
 *
 * @param should_log True to turn on logging all calls, false to turn it off.
 */
UNREALSDK_CAPI void log_all_calls(bool should_log) UNREALSDK_CAPI_SUFFIX;

/**
 * @brief Makes the next unreal function call completely ignore hooks.
 * @note Typically used to avoid recursion when re-calling the hooked function.
 */
UNREALSDK_CAPI void inject_next_call(void) UNREALSDK_CAPI_SUFFIX;

/**
 * @brief Adds a hook.
 *
 * @param func The function to hook.
 * @param type Which type of hook to add.
 * @param identifier The hook identifier.
 * @param callback The callback to run when the hooked function is called.
 * @return True if successfully added, false if an identical hook already existed.
 */
bool add_hook(const std::wstring& func,
              Type type,
              const std::wstring& identifier,
              Callback* callback);

/**
 * @brief Checks if a hook exists.
 *
 * @param func The function to hook.
 * @param type Which type of hook to check.
 * @param identifier The hook identifier.
 * @return True if a hook with the given details exists.
 */
bool has_hook(const std::wstring& func, Type type, const std::wstring& identifier);

/**
 * @brief Removes a hook.
 *
 * @param func The function to hook.
 * @param type Which type of hook to remove.
 * @param identifier The hook identifier.
 * @return True if successfully removed, false if no hook with the given details exists.
 */
bool remove_hook(const std::wstring& func, Type type, const std::wstring& identifier);

namespace impl {  // These functions are only relevant when implementing a game hook

struct List;

/*
Processing hooks needs to be very optimized, thousands if not tens of thousands of functions calls
are made every second.

Extracting all the information we need when running a hook is not always trivial, and we may not
even be able to do so for every single call (have to assume functions like that are never hooked).

To deal with this, hook processing is split in three.

Firstly, call `preprocess_hook`. This does some basic logging (if required), and then determines if
the function is hooked. If it isn't, it returns `nullptr`, and calling code can early exit. If there
is, it returns the list of hooks, to be passed to the next step.

If there is a hook, calling code can then spend more time retrieving the remaining information,
before calling `run_hooks_of_type` using pre-hooks. This actually runs all the hooks, and returns
the logical or of their return values. It can then run the unreal function or block execution as
required.

Extracting the return value may not be trivial either, so the calling code can run `has_post_hooks`
to work out if to early exit again. If it does, it can spend a bit longer extracting it, then call
`run_hooks_of_type` with the two post-hook types.
*/

/**
 * @brief Preprocess a function call, to work out if to bother trying to run hooks on it.
 *
 * @param source The source of the call, used for logging.
 * @param func The function which was called.
 * @param obj The object which called the function.
 * @return A pointer to the relevant hook list, or nullptr if no hooks match.
 */
const List* preprocess_hook(const std::string& source,
                            const unreal::UFunction* func,
                            const unreal::UObject* obj);

/**
 * @brief Checks if a hook list contains any post hooks.
 *
 * @param list The hook list, retrieved from `preprocess_hook`.
 * @return True if the list contains post hooks.
 */
bool has_post_hooks(const List& list);

/**
 * @brief Runs all the hooks in a list which match the given type.
 *
 * @param list The hook list, retrieved from `preprocess_hook`.
 * @param type The type of hooks to run.
 * @param hook The hook details.
 * @return The logical or of the hooks' return values.
 */
bool run_hooks_of_type(const List& list, Type type, Details& hook);

}  // namespace impl

}  // namespace unrealsdk::hook_manager

#endif /* UNREALSDK_HOOK_MANAGER_H */
