#include "pch.h"

#include "game/game_hook.h"
#include "hook_manager.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/wrapped_args.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::hook_manager {

map hooks{};
bool log_all_calls = false;
bool inject_next_call = false;

/**
 * @brief Checks if a hook may be called, and logs the call if needed.
 *
 * @param source The source of the hook, used for logging.
 * @param func_name The path name of the function which was called.
 * @param obj The object which called the function.
 * @return True if a hook exists for this function and may be called.
 */
static bool check_hook_allowed_and_log_call(const std::string& source,
                                            const std::string& func_name,
                                            const UObject* obj) {
    if (log_all_calls) {
        LOG(HOOKS, "===== %s called =====", source.c_str());
        LOG(HOOKS, "Function: %s", func_name.c_str());
        LOG(HOOKS, "Object: %s", obj->get_path_name<char>().c_str());
    }

    if (inject_next_call) {
        inject_next_call = false;
        return false;
    }

    return hooks.count(func_name) != 0;
}

/**
 * @brief Runs all hooks on the given function and decides if it may continue.
 *
 * @param func The function which was called.
 * @param func_name The path name of the function which was called. Optimization since previous
 *                  calls need to generate the name anyway.
 * @param obj The object which called the function.
 * @param args The arguments passed to the function.
 * @return True if to block execution, false if to continue.
 */
static bool run_hooks(UFunction* func,
                      const std::string& func_name,
                      UObject* obj,
                      WrappedArgs& args) {
    // Grab a copy incase the hook removes itself from the list (which would invalidate the
    // iterator)
    auto hook_list_copy = hooks[func_name];

    bool block = false;
    for (const auto& [_, hook_function] : hook_list_copy) {
        try {
            if (hook_function(func, obj, args)) {
                block = true;
                // Deliberately don't break, so that other hook functions get a chance to run too
            }
        } catch (const std::exception& ex) {
            LOG(ERROR, "An exception occured during hook processing: %s", ex.what());
        }
    }

    return block;
}

bool process_event(UObject* obj, UFunction* func, WrappedArgs& args) {
    auto func_name = func->get_path_name<char>();
    if (!check_hook_allowed_and_log_call("ProcessEvent", func_name, obj)) {
        return false;
    }

    return run_hooks(func, func_name, obj, args);
}

bool call_function(UObject* obj, FFrame* stack, void* /*result*/, UFunction* func) {
    // NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

    const auto EXPR_TOKEN_END_FUNCTION_PARMS = 0x16;
    const auto PROPERTY_FLAG_RETURN_PARAM = 0x400;

    /*
    Early exit if we have no hook, so we don't mess with the stack.

    NOTE: UE4 needs this to avoid access violations for a few special functions, e.g.:
    ```
    // /Script/Engine.KismetArrayLibrary:Array_Get
    void UKismetArrayLibrary::Array_Get(TArray<int> TargetArray, int Index, int* Item);
    ```
    This function presents itself as only taking (arrays of) ints, but is actually generic. It uses
     a native function which reads the actual property types out of the bytecode, and essentially
     ignores the ones presented by the UFunction.

    There doesn't seem to be a good way to handle this generically, we would have to hardcode each
     one, so by early exiting it only becomes a problem if someone hooks the function directly.
    As of writing this, we only know about the kismet array library functions doing this, which
     probably aren't very interesting hooks, so deliberately choosing to ignore it to keep the
     implementation simpler.
    */

    auto func_name = func->get_path_name<char>();
    if (!check_hook_allowed_and_log_call("CallFunction", func_name, obj)) {
        return false;
    }

    auto frame =
        func->ParamsSize == 0 ? nullptr : static_cast<uint8_t*>(calloc(1, func->ParamsSize));
    auto original_code = stack->Code;

    for (auto prop = reinterpret_cast<UProperty*>(func->Children);
         *stack->Code != EXPR_TOKEN_END_FUNCTION_PARMS;
         prop = reinterpret_cast<UProperty*>(prop->Next)) {
        bool is_return_param = (prop->PropertyFlags & PROPERTY_FLAG_RETURN_PARAM) != 0;
        if (is_return_param) {
            continue;
        }

        game::fframe_step(stack, stack->Object,
                          frame == nullptr ? nullptr : frame + prop->Offset_Internal);
    }

    WrappedArgs args{func, frame};
    auto block_function = run_hooks(func, func_name, obj, args);

    if (frame != nullptr) {
        free(frame);
    }

    if (block_function) {
        stack->Code++;
    } else {
        stack->Code = original_code;
    }

    return block_function;

    // NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
}

}  // namespace unrealsdk::hook_manager
