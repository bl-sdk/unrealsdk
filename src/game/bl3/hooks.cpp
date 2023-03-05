#include "pch.h"

#include "game/bl3/bl3.h"
#include "hook_manager.h"
#include "memory.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/wrapped_args.h"

#if defined(UE4) && defined(ARCH_X64)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

using process_event_func = void(UObject* obj, UFunction* func, void* params);

static process_event_func* process_event_ptr;
void process_event_hook(UObject* obj, UFunction* func, void* params) {
    try {
        WrappedArgs args{func, params};
        if (hook_manager::process_event(obj, func, args)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the ProcessEvent hook: %s", ex.what());
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
        if (hook_manager::call_function(obj, stack, result, func)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the CallFunction hook: %s", ex.what());
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
