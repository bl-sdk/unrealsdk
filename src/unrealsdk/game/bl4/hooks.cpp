#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl4/bl4.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/locks.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/multi_sigscan.h"
#include "unrealsdk/unreal/structs/fframe.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2 && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

#pragma region CallFunction

namespace {

using call_function_func = void(UObject* obj, FFrame* stack, void* result, UFunction* func);
call_function_func* call_function_ptr;

// binfold seems good at picking up UObject::execVirtualFunction, it calls two functions, first is
// UObject::FindFunctionChecked, second is this
const constexpr Pattern<30> CALL_FUNCTION_SIG{
    "55"              // push rbp
    "41 57"           // push r15
    "41 56"           // push r14
    "41 55"           // push r13
    "41 54"           // push r12
    "56"              // push rsi
    "57"              // push rdi
    "53"              // push rbx
    "48 83 EC ??"     // sub rsp, 38
    "48 8D 6C 24 ??"  // lea rbp, [rsp+30]
    "4C 89 C?"        // mov rbx, r9        | mov rsi, r9
    "4D 89 C6"        // mov r14, r8
    "48 89 D7"        // mov rdi, rdx
};

}  // namespace
namespace bl4 {
constinit MultiPattern call_function_multi{CALL_FUNCTION_SIG};
}  // namespace bl4

namespace {

void call_function_hook(UObject* obj, FFrame* stack, void* result, UFunction* func) {
    try {
        auto data = hook_manager::impl::preprocess_hook(L"CallFunction", func, obj);
        if (data != nullptr) {
            WrappedStruct args{func};
            auto original_code = stack->extract_current_args(args);

            hook_manager::Details hook{.obj = obj,
                                       .args = &args,
                                       .ret = {func->find_return_param()},
                                       .func = {.func = func, .object = obj}};

            const bool block_execution =
                hook_manager::impl::run_hooks_of_type(data, hook_manager::Type::PRE, hook);

            if (block_execution) {
                stack->Code()++;
            } else {
                stack->Code() = original_code;
                call_function_ptr(obj, stack, result, func);
            }

            if (hook.ret.has_value()) {
                // Result is a pointer directly to where the property should go, remove the offset
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(result)
                                 - hook.ret.prop->Offset_Internal());
            }

            if (!hook_manager::impl::has_post_hooks(data)) {
                return;
            }

            if (hook.ret.prop != nullptr && !hook.ret.has_value() && !block_execution) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(result)
                                   - hook.ret.prop->Offset_Internal());
            }

            if (!block_execution) {
                hook_manager::impl::run_hooks_of_type(data, hook_manager::Type::POST, hook);
            }

            hook_manager::impl::run_hooks_of_type(data, hook_manager::Type::POST_UNCONDITIONAL,
                                                  hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the CallFunction hook: {}", ex.what());
    }

    call_function_ptr(obj, stack, result, func);
}

void locking_call_function_hook(UObject* obj, FFrame* stack, void* result, UFunction* func) {
    const locks::FunctionCall lock{};
    call_function_hook(obj, stack, result, func);
}

static_assert(std::is_same_v<decltype(call_function_hook), call_function_func>,
              "call_function signature is incorrect");
static_assert(std::is_same_v<decltype(locking_call_function_hook), call_function_func>,
              "call_function signature is incorrect");

}  // namespace

void BL4Hook::hook_call_function(void) {
    detour(CALL_FUNCTION_SIG,
           locks::FunctionCall::enabled() ? locking_call_function_hook : call_function_hook,
           &call_function_ptr, "CallFunction");
}

#pragma endregion

#pragma region ProcessEvent

namespace {

using process_event_func = void(UObject* obj, UFunction* func, void* params);
process_event_func* process_event_ptr;

const constexpr Pattern<61> PROCESS_EVENT_SIG{
    "55"                    // push rbp
    "41 57"                 // push r15
    "41 56"                 // push r14
    "41 55"                 // push r13
    "41 54"                 // push r12
    "56"                    // push rsi
    "57"                    // push rdi
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 000000C8
    "48 8D AC 24 ????????"  // lea rbp, [rsp+80h]
    "48 8B 05 ????????"     // mov rax, [rip+0]
    "48 31 E8"              // xor rax, rbp
    "48 89 45 40"           // mov [rbp+40h], rax
    "48 85 C9"              // test rcx, rcx
    "0F 84 ????????"        // je Borderlands4.exe+D50E1
    "48 89 CF"              // mov rdi, rcx
    "8B 41 08"              // mov eax, [rcx+08]
    "A9 00000040"           // test eax, 40000000
};

}  // namespace
namespace bl4 {
constinit MultiPattern process_event_multi{PROCESS_EVENT_SIG};
}  // namespace bl4

namespace {

void process_event_hook(UObject* obj, UFunction* func, void* params) {
    try {
        auto data = hook_manager::impl::preprocess_hook(L"ProcessEvent", func, obj);
        if (data != nullptr) {
            // Copy args so that hooks can't modify them, for parity with call function
            const WrappedStruct args_base{func, params};
            WrappedStruct args = args_base.copy_params_only();
            hook_manager::Details hook{.obj = obj,
                                       .args = &args,
                                       .ret = {func->find_return_param()},
                                       .func = {.func = func, .object = obj}};

            const bool block_execution =
                hook_manager::impl::run_hooks_of_type(data, hook_manager::Type::PRE, hook);

            if (!block_execution) {
                process_event_ptr(obj, func, params);
            }

            if (hook.ret.has_value()) {
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(params));
            }

            if (!hook_manager::impl::has_post_hooks(data)) {
                return;
            }

            if (hook.ret.prop != nullptr && !hook.ret.has_value() && !block_execution) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(params));
            }

            if (!block_execution) {
                hook_manager::impl::run_hooks_of_type(data, hook_manager::Type::POST, hook);
            }

            hook_manager::impl::run_hooks_of_type(data, hook_manager::Type::POST_UNCONDITIONAL,
                                                  hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the ProcessEvent hook: {}", ex.what());
    }

    process_event_ptr(obj, func, params);
}

void locking_process_event_hook(UObject* obj, UFunction* func, void* params) {
    const locks::FunctionCall lock{};
    process_event_hook(obj, func, params);
}

static_assert(std::is_same_v<decltype(process_event_hook), process_event_func>,
              "process_event signature is incorrect");
static_assert(std::is_same_v<decltype(process_event_hook), decltype(locking_process_event_hook)>,
              "process_event signature is incorrect");

}  // namespace

void BL4Hook::hook_process_event_and_wait_for_unpack(void) {
    // HACK: The exe is packed, we don't have a proper hook after unpack yet, so instead just repeat
    //       sigscan until it works (or timeout). This is normally the first sigscan.
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds{30};
    while (std::chrono::steady_clock::now() < timeout) {
        auto process_event_addr = PROCESS_EVENT_SIG.sigscan_nullable();
        if (process_event_addr == 0) {
            const constexpr auto sleep_time = std::chrono::milliseconds{50};
            std::this_thread::sleep_for(sleep_time);
            continue;
        }

        detour(process_event_addr,
               locks::FunctionCall::enabled() ? locking_process_event_hook : process_event_hook,
               &process_event_ptr, "ProcessEvent");
        return;
    }

    LOG(ERROR, "Couldn't find ProcessEvent signature before timeout!");
}

void BL4Hook::process_event(UObject* object, UFunction* func, void* params) const {
    // When we call it manually, always call the locking version, it will pass right through if
    // locks are disabled
    locking_process_event_hook(object, func, params);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
