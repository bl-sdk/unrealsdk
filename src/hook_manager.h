#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include "pch.h"
#include "unreal/wrappers/bound_function.h"
#include "unreal/wrappers/property_proxy.h"

namespace unrealsdk::unreal {

class UObject;
class UFunction;

class WrappedStruct;

}  // namespace unrealsdk::unreal

namespace unrealsdk::hook_manager {

struct HookDetails {
    unreal::UObject* obj{};
    unreal::WrappedStruct& args;
    unreal::PropertyProxy ret{nullptr};
    unreal::BoundFunction func{};
};

using Callback = bool(HookDetails&);

using HookGroup = std::unordered_map<std::wstring, std::function<Callback>>;

struct HookList {
    HookGroup pre;
    HookGroup post;

    /**
     * @brief Checks if all groups in the list are empty.
     *
     * @return True if all groups are empty
     */
    [[nodiscard]] bool empty(void) const;
};

using HookMap = std::unordered_map<std::wstring, HookList>;

/*
To register a hook, simply create a function matching the signature of `hook_manager::Callback`, and
add it to the hooks map:

```
hook_manager::hooks[FUNCTION_NAME].pre[IDENTIFIER] = func;
```

`FUNCTION_NAME` is the full unreal function name.

Hooks can be added to either the `pre` or `post` groups, to run before or after the hooked function
respectively. There are a few more minor distinctions discussed later.

`IDENTIFIER` is an extra unique id, required because `std::function` doesn't have proper equality or
hashing. This would make it difficult to later remove the hook. Strings are chosen for this since
they're more likely to be unique.

To remove a hook, simply remove it from the map.


When an unreal function is called, the matching hook functions are run.

The pre-hooks are always run before the function, and the post-hooks after, but the order hooks are
run within each group is undefined.

Hooks are called with 4 args (wrapped in a struct):
`obj`  | The object the hooked function was called on.
`args` | The arguments the hooked function was called with. While this is mutable, modifying it
       | will *not* modify the actual function arguments.
`ret`  | A proxy for the return value. More on this later.
`func` | The function which was called, bound to the same object. Can be used to re-call it.

Hooks return a boolean. If any pre-hook returns true, execution of the unreal function and the
post-hooks is blocked. The return value is ignored for post-hooks.

During pre-hooks, `ret` is an unset value, and setting it will overwrite the return value of the
unreal function call. The value set when pre-hook processing is complete will be used, if multiple
hooks all want to overwrite it they will need to cooperate.

During post-hooks, `ret` is set to the return value of the unreal function. This may have been
overwritten by a pre-hook. If execution was blocked and it was not overwritten during a pre-hook, it
will be an unset value. Note that while there may be a property `args.ReturnValue`, it is not
necessarily correct, `ret` always will be.
*/

/// The hook map.
extern HookMap hooks;
/// If true, every unreal function call will be logged. Best used in short bursts for debugging.
extern bool log_all_calls;
/// If true, hooks will completely ignore the next unreal function call. Automatically resets
/// afterwards. Typically used to avoid recursion when re-calling the hooked function.
extern bool inject_next_call;

// =================================================================================================

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
before passing one of the groups to `run_hook_group`. This actually runs all the hooks, and returns
the logical or of their return values.
*/

/**
 * @brief Preprocess a function call, to work out if to bother trying to run hooks on it.
 *
 * @param source The source of the call, used for logging.
 * @param func The function which was called.
 * @param obj The object which called the function.
 * @return A pointer to the relevant hook group, or nullptr if no hooks match.
 */
const HookList* preprocess_hook(const std::string& source,
                                const unreal::UFunction* func,
                                const unreal::UObject* obj);

/**
 * @brief Runs all hooks in a group.
 *
 * @param group The group of hooks to run, retrieved from `preprocess_hook`.
 * @param hook The hook details.
 * @return The logical or of the hooks' return values.
 */
bool run_hook_group(const HookGroup& group, HookDetails& hook);

}  // namespace unrealsdk::hook_manager

#endif /* HOOK_MANAGER_H */
