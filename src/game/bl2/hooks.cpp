#include "pch.h"

#include "game/bl2/bl2.h"
#include "hook_manager.h"
#include "memory.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/wrappers/wrapped_args.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

// This function is actually thiscall, but MSVC won't let us declare static thiscall functions
// As a workaround, declare it fastcall, and add a dummy edx arg.
// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* process_event_func)(UObject* obj,
                                             void* /*edx*/,
                                             UFunction* func,
                                             void* params,
                                             void* result);

static process_event_func process_event_ptr;
static void __fastcall process_event_hook(UObject* obj,
                                          void* edx,
                                          UFunction* func,
                                          void* params,
                                          void* result) {
    try {
        WrappedArgs args{func, params};
        if (hook_manager::process_event(obj, func, args)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the ProcessEvent hook: {}", ex.what());
    }

    process_event_ptr(obj, edx, func, params, result);
}
static_assert(std::is_same_v<decltype(&process_event_hook), process_event_func>,
              "process_event signature is incorrect");

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

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* call_function_func)(UObject* obj,
                                             void* /*edx*/,
                                             FFrame* stack,
                                             void* params,
                                             UFunction* func);

static call_function_func call_function_ptr;
static void __fastcall call_function_hook(UObject* obj,
                                          void* edx,
                                          FFrame* stack,
                                          void* result,
                                          UFunction* func) {
    try {
        if (hook_manager::call_function(obj, stack, result, func)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the CallFunction hook: {}", ex.what());
    }

    call_function_ptr(obj, edx, stack, result, func);
}
static_assert(std::is_same_v<decltype(&call_function_hook), call_function_func>,
              "call_function signature is incorrect");

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
