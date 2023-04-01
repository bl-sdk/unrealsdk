#include "pch.h"

#include "game/bl3/bl3.h"
#include "hook_manager.h"
#include "memory.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/wrapped_struct.h"

#if defined(UE4) && defined(ARCH_X64)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

using process_event_func = void(UObject* obj, UFunction* func, void* params);

static process_event_func* process_event_ptr;
void process_event_hook(UObject* obj, UFunction* func, void* params) {
    try {
        auto list = hook_manager::preprocess_hook("ProcessEvent", func, obj);
        if (list != nullptr) {
            // Copy args so that hooks can't modify them, for parity with call function
            WrappedStruct args_base{func, params};
            WrappedStruct args(args_base);
            hook_manager::HookDetails hook{obj, args, {func->find_return_param()}, {func, obj}};
            bool block_execution = hook_manager::process_hook(*list, hook);

            if (!block_execution) {
                process_event_ptr(obj, func, params);
            }

            if (hook.ret.has_value()) {
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(params));
            }

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the ProcessEvent hook: {}", ex.what());
    }

    process_event_ptr(obj, func, params);
}
static_assert(std::is_same_v<decltype(process_event_hook), process_event_func>,
              "process_event signature is incorrect");

void BL3Hook::hook_process_event(void) {
    const Pattern PROCESS_EVENT_SIG{
        "\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\xF0\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    sigscan_and_detour(PROCESS_EVENT_SIG, process_event_hook, &process_event_ptr, "ProcessEvent");
}

void BL3Hook::process_event(UObject* object, UFunction* func, void* params) const {
    process_event_hook(object, func, params);
}

using call_function_func = void(UObject* obj, FFrame* stack, void* result, UFunction* func);

static call_function_func* call_function_ptr;
void call_function_hook(UObject* obj, FFrame* stack, void* result, UFunction* func) {
    try {
        /*
        NOTE: The early exit here also avoids access violations for a few special functions, e.g.:
        ```
        // /Script/Engine.KismetArrayLibrary:Array_Get
        void UKismetArrayLibrary::Array_Get(TArray<int> TargetArray, int Index, int* Item);
        ```

        This function presents itself as only taking (arrays of) ints, but is actually generic. It
        uses a native function which reads the actual property types out of the bytecode, and
        essentially ignores the ones presented by the UFunction.

        This means when we try extract args, the sizes don't line up, so we almost always get an
        access violation.

        There doesn't seem to be a good way to handle this generically, we would have to hardcode
        each function, so by early exiting it only becomes a problem if someone hooks the function
        directly.

        As of writing this, we only know about the kismet array library functions doing this, which
        probably aren't very interesting hooks, so deliberately choosing to ignore it to keep the
        implementation simpler.
        */

        auto list = hook_manager::preprocess_hook("ProcessEvent", func, obj);
        if (list != nullptr) {
            WrappedStruct args{func};
            auto original_code = stack->extract_current_args(args);

            hook_manager::HookDetails hook{obj, args, {func->find_return_param()}, {func, obj}};
            bool block_execution = hook_manager::process_hook(*list, hook);

            if (block_execution) {
                stack->Code++;
            } else {
                stack->Code = original_code;
                call_function_ptr(obj, stack, result, func);
            }

            if (hook.ret.has_value()) {
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(result));
            }

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the CallFunction hook: {}", ex.what());
    }

    call_function_ptr(obj, stack, result, func);
}
static_assert(std::is_same_v<decltype(call_function_hook), call_function_func>,
              "call_function signature is incorrect");

void BL3Hook::hook_call_function(void) {
    const Pattern CALL_FUNCTION_SIG{
        "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xec\x28\x01\x00\x00\x48\x8d"
        "\x6c\x24\x30",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF"};

    sigscan_and_detour(CALL_FUNCTION_SIG, call_function_hook, &call_function_ptr, "CallFunction");
}

}  // namespace unrealsdk::game

#endif
