#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/property_proxy.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

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

const Pattern PROCESS_EVENT_SIG{
    "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x50\xA1"
    "\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00"
    "\x00\x8B\xF1\x89\x75\xEC",
    "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF"
    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"
    "\x00\xFF\xFF\xFF\xFF\xFF"};

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

        auto data = hook_manager::impl::preprocess_hook("ProcessEvent", func, obj);
        if (data != nullptr) {
            // Copy args so that hooks can't modify them, for parity with call function
            const WrappedStruct args_base{func, params};
            WrappedStruct args(args_base);
            hook_manager::Details hook{obj, &args, {func->find_return_param()}, {func, obj}};

            const bool block_execution =
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::PRE, hook);

            if (!block_execution) {
                process_event_ptr(obj, edx, func, params, null);
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

    process_event_ptr(obj, edx, func, params, null);
}
static_assert(std::is_same_v<decltype(&process_event_hook), process_event_func>,
              "process_event signature is incorrect");

}  // namespace

void BL2Hook::hook_process_event(void) {
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

const Pattern CALL_FUNCTION_SIG{
    "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00"
    "\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3"
    "\x00\x00\x00\x00\x8B\x7D\x10\x8B\x45\x0C",
    "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"
    "\x00\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00"
    "\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

void __fastcall call_function_hook(UObject* obj,
                                   void* edx,
                                   FFrame* stack,
                                   void* result,
                                   UFunction* func) {
    try {
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
                call_function_ptr(obj, edx, stack, result, func);
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

    call_function_ptr(obj, edx, stack, result, func);
}
static_assert(std::is_same_v<decltype(&call_function_hook), call_function_func>,
              "call_function signature is incorrect");

}  // namespace

void BL2Hook::hook_call_function(void) {
    sigscan_and_detour(CALL_FUNCTION_SIG, call_function_hook, &call_function_ptr, "CallFunction");
}

}  // namespace unrealsdk::game

#endif
