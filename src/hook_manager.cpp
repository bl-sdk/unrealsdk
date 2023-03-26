#include "pch.h"

#include "hook_manager.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/wrapped_args.h"
#include "unrealsdk.h"
#include "utils.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::hook_manager {

map hooks{};
bool log_all_calls = false;
bool inject_next_call = false;

/**
  * @brief Preprocess a function call to work out if to bother trying to run hooks on it.
  *
  * @param source The source of the hook, used for logging.
  * @param func The function which was called.
  * @param obj The object which called the function.
  * @return A pair, of true and the function name if a hook may be called, or of false and an
  *         unspecified string otherwise.
  */
static std::pair<bool, std::wstring> preprocess_hook(const std::string& source,
                                                     const UFunction* func,
                                                     const UObject* obj) {
    if (inject_next_call) {
        inject_next_call = false;
        return {false, L""};
    }

    auto func_name = func->get_path_name();

    if (log_all_calls) {
        LOG(MISC, "===== {} called =====", source);
        LOG(MISC, "Function: {}", func_name);
        LOG(MISC, "Object: {}", obj->get_path_name());
    }

    return {hooks.contains(func_name), func_name};
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
                      const std::wstring& func_name,
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
            LOG(ERROR, "An exception occured during hook processing: {}", ex.what());
        }
    }

    return block;
}

bool process_event(UObject* obj, UFunction* func, WrappedArgs& args) {
    auto [has_hook, func_name] = preprocess_hook("ProcessEvent", func, obj);
    if (!has_hook) {
        return false;
    }

    return run_hooks(func, func_name, obj, args);
}

bool call_function(UObject* obj, FFrame* stack, void* /*result*/, UFunction* func) {
    // NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

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

    auto [has_hook, func_name] = preprocess_hook("CallFunction", func, obj);
    if (!has_hook) {
        return false;
    }

    auto frame = func->ParamsSize == 0
                     ? nullptr
                     : static_cast<uint8_t*>(unrealsdk::u_malloc(func->ParamsSize));
    auto original_code = stack->Code;

    for (auto prop = reinterpret_cast<UProperty*>(func->Children);
         *stack->Code != FFrame::EXPR_TOKEN_END_FUNCTION_PARMS;
         prop = reinterpret_cast<UProperty*>(prop->Next)) {
        bool is_return_param = (prop->PropertyFlags & UProperty::PROP_FLAG_RETURN) != 0;
        if (is_return_param) {
            continue;
        }

        unrealsdk::fframe_step(stack, stack->Object,
                               frame == nullptr ? nullptr : frame + prop->Offset_Internal);
    }

    WrappedArgs args{func, frame};
    auto block_function = run_hooks(func, func_name, obj, args);

    if (frame != nullptr) {
        unrealsdk::u_free(frame);
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
