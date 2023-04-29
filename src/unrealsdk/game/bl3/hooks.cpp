#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

namespace {

using process_event_func = void(UObject* obj, UFunction* func, void* params);
process_event_func* process_event_ptr;

const Pattern PROCESS_EVENT_SIG{
    "\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\xF0\x00\x00\x00",
    "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

}  // namespace

void process_event_hook(UObject* obj, UFunction* func, void* params) {
    try {
        auto data = hook_manager::impl::preprocess_hook("ProcessEvent", func, obj);
        if (data != nullptr) {
            // Copy args so that hooks can't modify them, for parity with call function
            const WrappedStruct args_base{func, params};
            WrappedStruct args(args_base);
            hook_manager::Details hook{obj, &args, {func->find_return_param()}, {func, obj}};

            const bool block_execution =
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::PRE, hook);

            if (!block_execution) {
                process_event_ptr(obj, func, params);
            }

            if (hook.ret.has_value()) {
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(params));
            }

            if (!hook_manager::impl::has_post_hooks(*data)) {
                return;
            }

            if (hook.ret.prop != nullptr && !hook.ret.has_value() && !block_execution) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(params));
            }

            if (!block_execution) {
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST, hook);
            }

            hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST_UNCONDITIONAL,
                                                  hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the ProcessEvent hook: {}", ex.what());
    }

    process_event_ptr(obj, func, params);
}
static_assert(std::is_same_v<decltype(process_event_hook), process_event_func>,
              "process_event signature is incorrect");

void BL3Hook::hook_process_event(void) {
    sigscan_and_detour(PROCESS_EVENT_SIG, process_event_hook, &process_event_ptr, "ProcessEvent");
}

void BL3Hook::process_event(UObject* object, UFunction* func, void* params) const {
    process_event_hook(object, func, params);
}

namespace {

using call_function_func = void(UObject* obj, FFrame* stack, void* result, UFunction* func);
call_function_func* call_function_ptr;

const Pattern CALL_FUNCTION_SIG{
    "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xec\x28\x01\x00\x00\x48\x8d"
    "\x6c\x24\x30",
    "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
    "\xFF\xFF\xFF"};

}  // namespace

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

        auto data = hook_manager::impl::preprocess_hook("ProcessEvent", func, obj);
        if (data != nullptr) {
            WrappedStruct args{func};
            auto original_code = stack->extract_current_args(args);

            hook_manager::Details hook{obj, &args, {func->find_return_param()}, {func, obj}};

            const bool block_execution =
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::PRE, hook);

            if (block_execution) {
                stack->Code++;
            } else {
                stack->Code = original_code;
                call_function_ptr(obj, stack, result, func);
            }

            if (hook.ret.has_value()) {
                // Result is a pointer directly to where the property should go, remove the offset
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(result)
                                 - hook.ret.prop->Offset_Internal);
            }

            if (!hook_manager::impl::has_post_hooks(*data)) {
                return;
            }

            if (hook.ret.prop != nullptr && !hook.ret.has_value() && !block_execution) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(result)
                                   - hook.ret.prop->Offset_Internal);
            }

            if (!block_execution) {
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST, hook);
            }

            hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST_UNCONDITIONAL,
                                                  hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the CallFunction hook: {}", ex.what());
    }

    call_function_ptr(obj, stack, result, func);
}
static_assert(std::is_same_v<decltype(call_function_hook), call_function_func>,
              "call_function signature is incorrect");

void BL3Hook::hook_call_function(void) {
    sigscan_and_detour(CALL_FUNCTION_SIG, call_function_hook, &call_function_ptr, "CallFunction");
}

}  // namespace unrealsdk::game

#endif
