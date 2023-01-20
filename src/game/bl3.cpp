#include "pch.h"

#if defined(UE4) && defined(ARCH_X64)

#include "game/bl3.h"
#include "game/game_hook.h"
#include "hook_manager.h"
#include "memory.h"
#include "unreal/classes/ufunction.h"
#include "unreal/structs/fframe.h"
#include "unreal/structs/fname.h"
#include "unreal/wrappers/gobjects.h"
#include "unreal/wrappers/wrappedargs.h"

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

#pragma region ProcessEvent

using process_event_func = void(UObject* obj, UFunction* func, void* params);

static process_event_func* process_event_ptr;
void process_event_hook(UObject* obj, UFunction* func, void* params) {
    try {
        WrappedArgs args{func, params};
        if (hook_manager::process_event(obj, func, args)) {
            return;
        }
    } catch (const std::exception &ex) {
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

#pragma endregion

#pragma region CallFunction

using call_function_func = void(UObject* obj, FFrame* stack, void* result, UFunction* func);

static call_function_func* call_function_ptr;
void call_function_hook(UObject* obj, FFrame* stack, void* result, UFunction* func) {
    try {
        if (hook_manager::call_function(obj, stack, result, func)) {
            return;
        }
    } catch (const std::exception &ex) {
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

#pragma endregion

#pragma region Globals

void BL3Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{
        "\x48\x8D\x0D\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\xE8\x00\x00\x00\x00\xC6\x05",
        "\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF", 3};

    auto gobjects_instr = sigscan(GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);

    this->gobjects = GObjects(gobjects_ptr);
}

void BL3Hook::find_gnames(void) {
    static const Pattern GNAMES_SIG{
        "\xE8\x00\x00\x00\x00\x48\x00\x00\x48\x89\x1D\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83"
        "\xC4\x28\xC3\x00\xDB\x48\x89\x1D\x00\x00\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83\xC4"
        "\x00\xC3",
        "\xFF\x00\x00\x00\x00\xFF\x00\x00\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF"
        "\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF"
        "\x00\xFF",
        0xB};

    auto gnames_instr = sigscan(GNAMES_SIG);
    auto gnames_ptr = *read_offset<GNames::internal_type*>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    this->gnames = GNames(gnames_ptr);
}

#pragma endregion

#pragma region FName::Init

void BL3Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_PATTERN{
        "\x40\x53\x48\x83\xEC\x30\xC7\x44\x24\x00\x00\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00"};

    this->fname_init_ptr = sigscan<fname_init_func*>(FNAME_INIT_PATTERN);
    LOG(MISC, "FName::Init: 0x%p", this->fname_init_ptr);
}

void BL3Hook::fname_init(FName* name, const std::wstring& str, int32_t number) const {
    this->fname_init(name, str.c_str(), number);
}
void BL3Hook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    *name = this->fname_init_ptr(str, number, 1);
}

#pragma endregion

#pragma region FFrame::Step

void BL3Hook::find_fframe_step(void) {
    static const Pattern FFRAME_STEP_SIG{"\x48\x8B\x41\x20\x4C\x8B\xD2\x48\x8B\xD1",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    this->fframe_step_ptr = sigscan<fframe_step_func*>(FFRAME_STEP_SIG);
    LOG(MISC, "FFrame::Step: 0x%p", this->fframe_step_ptr);
}
void BL3Hook::fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const {
    this->fframe_step_ptr(frame, obj, param);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
