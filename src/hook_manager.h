#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include "pch.h"
#include "unreal/wrappers/bound_function.h"
#include "unreal/wrappers/property_proxy.h"

namespace unrealsdk::unreal {

class UObject;
class UFunction;

class ReadOnlyWrappedStruct;

}  // namespace unrealsdk::unreal

namespace unrealsdk::hook_manager {

struct HookDetails {
    unreal::UObject* obj{};
    unreal::ReadOnlyWrappedStruct& args;
    unreal::PropertyProxy ret{nullptr};
    unreal::BoundFunction func{};
};

using callback = bool(HookDetails&);
using func_name = std::wstring;
using identifier = std::wstring;

using list = std::unordered_map<identifier, std::function<callback>>;
using map = std::unordered_map<func_name, list>;

/*
To register a hook, simply create a function matching the signature of `hook_manager::callback`, and
add it to the hooks map. The outer key is the full unreal function name, and the inner key is an
arbitrary unique identifier.

An inner key is required because `std::function` doesn't have proper equality or hashing. This makes
it difficult to later remove the hook.

If multiple hooks match the same unreal function, they will all be run, but in an undefined order.

If any hook returns true, execution of the unreal function will be blocked.

If `HookDetails::ret` has a value after all hooks are run, the return value of the unreal function
will be overwritten with it. If multiple hooks try modify the return value, they will have to
cooperate, since which will run last is undefined.
*/

extern map hooks;
extern bool log_all_calls;
extern bool inject_next_call;

/*
Processing hooks needs to be very optimized, thousands if not tens of thousands of functions calls
are made every second.

Extracting all the information we need when running a hook is not always trivial, and we may not
even be able to do so for every single call (have to assume function like that are never hooked).

To deal with this, hook processing is split in two.

Firstly, call `preprocess_hook`. This does some basic logging (if required), and then determines if
the function is hooked. If it isn't, it returns `nullptr`, and calling code can early exit. If there
is, it returns the list of hooks, to be passed to the next step.

If there is a hook, calling code can then spend more time retrieving the remaining information,
before passing it to `process_hook`. This actually runs all the hooks, which may then modify the
return value, or mark that execution should be blocked.
*/

/**
 * @brief Preprocess a function call, to work out if to bother trying to run hooks on it.
 *
 * @param source The source of the call, used for logging.
 * @param func The function which was called.
 * @param obj The object which called the function.
 * @return A pointer to a hook list, or nullptr if no hooks match.
 */
const list* preprocess_hook(const std::string& source,
                            const unreal::UFunction* func,
                            const unreal::UObject* obj);

/**
 * @brief Runs all hooks on a function.
 * @note Caller should examine both the return value and `hook.ret` to determine what to do with the
 *       hooked call.
 *
 * @param list The list of hooks retrieved from `preprocess_hook`.
 * @param hook The hook details.
 * @return True if the hook should block execution, false otherwise.
 */
bool process_hook(const list& list, HookDetails& hook);

}  // namespace unrealsdk::hook_manager

#endif /* HOOK_MANAGER_H */
