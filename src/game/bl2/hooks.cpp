#include "pch.h"
#include <stdexcept>

#include "game/bl2/bl2.h"
#include "hook_manager.h"
#include "memory.h"
#include "unreal/cast_prop.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/classes/uproperty.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/bound_function.h"
#include "unreal/wrappers/property_proxy.h"
#include "unreal/wrappers/wrapped_struct.h"
#include "unrealsdk.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

// This function is actually thiscall, but MSVC won't let us declare static thiscall functions
// As a workaround, declare it fastcall, and add a dummy edx arg.
// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* process_event_func)(UObject* obj,
                                             void* /*edx*/,
                                             UFunction* func,
                                             void* params,
                                             void* /*null*/);

process_event_func process_event_ptr;
void __fastcall process_event_hook(UObject* obj,
                                   void* edx,
                                   UFunction* func,
                                   void* params,
                                   void* null) {
    try {
        // This arg seems to be in the process of being deprecated, no usage in ghidra, always seems
        // to be null, and it's gone in later ue versions. Gathering some extra info just in case.
        if (null != nullptr) {
            LOG(DEV_WARNING, L"Null param had a value in process event during func {} on obj {}",
                func->get_path_name(), obj->get_path_name());
        }

        auto list = hook_manager::preprocess_hook("ProcessEvent", func, obj);
        if (list != nullptr) {
            // Copy args so that hooks can't modify them, for parity with call function
            const WrappedStruct ARGS_BASE{func, params};
            WrappedStruct args(ARGS_BASE);
            hook_manager::HookDetails hook{obj, &args, {func->find_return_param()}, {func, obj}};

            const bool BLOCK_EXECUTION = hook_manager::run_hook_group(list->pre, hook);

            if (!BLOCK_EXECUTION) {
                process_event_ptr(obj, edx, func, params, null);
            }

            if (hook.ret.has_value()) {
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(params));
            }

            if (list->post.empty()) {
                return;
            }

            if (!hook.ret.has_value() && !BLOCK_EXECUTION) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(params));
            }

            hook_manager::run_hook_group(list->post, hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the ProcessEvent hook: {}", ex.what());
    }

    process_event_ptr(obj, edx, func, params, null);
}
static_assert(std::is_same_v<decltype(&process_event_hook), process_event_func>,
              "process_event signature is incorrect");

}  // namespace

void BL2Hook::hook_process_event(void) {
    const Pattern PROCESS_EVENT_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x50\xA1"
        "\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00"
        "\x00\x8B\xF1\x89\x75\xEC",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF"
        "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"
        "\x00\xFF\xFF\xFF\xFF\xFF"};

    sigscan_and_detour(PROCESS_EVENT_SIG, process_event_hook, &process_event_ptr, "ProcessEvent");
}

void BL2Hook::process_event(UObject* object, UFunction* func, void* params) const {
    process_event_hook(object, nullptr, func, params, nullptr);
}

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* call_function_func)(UObject* obj,
                                             void* /*edx*/,
                                             FFrame* stack,
                                             void* params,
                                             UFunction* func);

call_function_func call_function_ptr;
void __fastcall call_function_hook(UObject* obj,
                                   void* edx,
                                   FFrame* stack,
                                   void* result,
                                   UFunction* func) {
    try {
        auto list = hook_manager::preprocess_hook("ProcessEvent", func, obj);
        if (list != nullptr) {
            WrappedStruct args{func};
            auto original_code = stack->extract_current_args(args);

            hook_manager::HookDetails hook{obj, &args, {func->find_return_param()}, {func, obj}};

            const bool BLOCK_EXECUTION = hook_manager::run_hook_group(list->pre, hook);

            if (BLOCK_EXECUTION) {
                stack->Code++;
            } else {
                stack->Code = original_code;
                call_function_ptr(obj, edx, stack, result, func);
            }

            if (hook.ret.has_value()) {
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(result));
            }

            if (list->post.empty()) {
                return;
            }

            if (!hook.ret.has_value() && !BLOCK_EXECUTION) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(result));
            }

            hook_manager::run_hook_group(list->post, hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the CallFunction hook: {}", ex.what());
    }

    call_function_ptr(obj, edx, stack, result, func);
}
static_assert(std::is_same_v<decltype(&call_function_hook), call_function_func>,
              "call_function signature is incorrect");

}  // namespace

void BL2Hook::hook_call_function(void) {
    const Pattern CALL_FUNCTION_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00"
        "\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3"
        "\x00\x00\x00\x00\x8B\x7D\x10\x8B\x45\x0C",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"
        "\x00\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00"
        "\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    sigscan_and_detour(CALL_FUNCTION_SIG, call_function_hook, &call_function_ptr, "CallFunction");
}

}  // namespace unrealsdk::game

#endif
