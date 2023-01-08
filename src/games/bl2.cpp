#include "pch.h"

#include "games/bl2.h"
#include "games/game_hook.h"
#include "hook_manager.h"
#include "sigscan.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/wrappers/gobjects.h"
#include "unreal/wrappers/wrappedargs.h"

using namespace unrealsdk::sigscan;
using namespace unrealsdk::unreal;

namespace unrealsdk::games {

#pragma region AntiDebug

// NOLINTBEGIN(readability-identifier-naming)
// NOLINTNEXTLINE(modernize-use-using)  - need a typedef for calling conventions in msvc
typedef NTSTATUS(WINAPI* NtSetInformationThread_func)(
    HANDLE ThreadHandle,
    THREAD_INFORMATION_CLASS ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength);
// NOLINTNEXTLINE(modernize-use-using)
typedef NTSTATUS(WINAPI* NtQueryInformationProcess_func)(HANDLE ProcessHandle,
                                                         PROCESSINFOCLASS ProcessInformationClass,
                                                         PVOID ProcessInformation,
                                                         ULONG ProcessInformationLength,
                                                         PULONG ReturnLength);

static constexpr auto ThreadHideFromDebugger = static_cast<THREAD_INFORMATION_CLASS>(17);
static constexpr auto ProcessDebugObjectHandle = static_cast<PROCESSINFOCLASS>(30);
// NOLINTEND(readability-identifier-naming)

// NOLINTBEGIN(readability-identifier-naming)
static NtSetInformationThread_func NtSetInformationThread_ptr;
static NTSTATUS NTAPI NtSetInformationThread_hook(HANDLE ThreadHandle,
                                                  THREAD_INFORMATION_CLASS ThreadInformationClass,
                                                  PVOID ThreadInformation,
                                                  ULONG ThreadInformationLength) {
    // NOLINTEND(readability-identifier-naming)
    if (ThreadInformationClass == ThreadHideFromDebugger) {
        return STATUS_SUCCESS;
    }

    return NtSetInformationThread_ptr(ThreadHandle, ThreadInformationClass, ThreadInformation,
                                      ThreadInformationLength);
}
static_assert(std::is_same_v<decltype(&NtSetInformationThread_hook), NtSetInformationThread_func>,
              "NtSetInformationThread signature is incorrect");

// NOLINTBEGIN(readability-identifier-naming)
static NtQueryInformationProcess_func NtQueryInformationProcess_ptr;
static NTSTATUS WINAPI NtQueryInformationProcess_hook(HANDLE ProcessHandle,
                                                      PROCESSINFOCLASS ProcessInformationClass,
                                                      PVOID ProcessInformation,
                                                      ULONG ProcessInformationLength,
                                                      PULONG ReturnLength) {
    // NOLINTEND(readability-identifier-naming)
    if (ProcessInformationClass == ProcessDebugObjectHandle) {
        return STATUS_PORT_NOT_SET;
    }

    return NtQueryInformationProcess_ptr(ProcessHandle, ProcessInformationClass, ProcessInformation,
                                         ProcessInformationLength, ReturnLength);
}
static_assert(
    std::is_same_v<decltype(&NtQueryInformationProcess_hook), NtQueryInformationProcess_func>,
    "NtQueryInformationProcess signature is incorrect");

void BL2Hook::hook_antidebug(void) {
    MH_STATUS status = MH_OK;

    LPVOID target = nullptr;
    status = MH_CreateHookApiEx(L"ntdll", "NtSetInformationThread",
                                reinterpret_cast<LPVOID>(NtSetInformationThread_hook),
                                reinterpret_cast<LPVOID*>(&NtSetInformationThread_ptr), &target);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create NtSetInformationThread hook: %x", status);
    } else {
        status = MH_EnableHook(target);
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable NtSetInformationThread hook: %x", status);
        }
    }

    status = MH_CreateHookApiEx(L"ntdll", "NtQueryInformationProcess",
                                reinterpret_cast<LPVOID>(NtQueryInformationProcess_hook),
                                reinterpret_cast<LPVOID*>(&NtQueryInformationProcess_ptr), &target);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create NtQueryInformationProcess hook: %x", status);
    } else {
        status = MH_EnableHook(target);
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable NtQueryInformationProcess hook: %x", status);
        }
    }
}

#pragma endregion

#pragma region ProcessEvent

// This function is actually thiscall, but MSVC won't let us declare static thiscall functions
// As a workaround, declare it fastcall, and add a dummy edx arg.
// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* process_event_func)(UObject* obj,
                                             void* /*edx*/,
                                             UFunction* func,
                                             void* params,
                                             void* result);

process_event_func process_event_ptr;
static void __fastcall process_event_hook(UObject* obj,
                                          void* edx,
                                          UFunction* func,
                                          void* params,
                                          void* result) {
    WrappedArgs args{func, params};
    if (hook_manager::process_hooks(func, obj, args, "ProcessEvent")) {
        return;
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

    scan_and_detour(this->start, this->size, PROCESS_EVENT_SIG, process_event_hook,
                    &process_event_ptr, "ProcessEvent");
}

#pragma endregion

/*
using type_CallFunction = __thiscall void(UObject* obj, FFrame* frame, void* res, UFunction* func);
const Pattern CALL_FUNCTION_SIG{
    "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00"
    "\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3"
    "\x00\x00\x00\x00\x8B\x7D\x10\x8B\x45\x0C",
    "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"
    "\x00\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00"
    "\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};
*/

void BL2Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x8B\x40\x08",
                                      "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gobjects_instr = scan(this->start, this->size, GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);

    this->gobjects = GObjects(gobjects_ptr);
}

void BL2Hook::find_gnames(void) {
    static const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x83\x3C\x81\x00\x74\x5C",
                                    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gnames_instr = scan(this->start, this->size, GNAMES_SIG);
    auto gnames_ptr = read_offset<GNames::internal_type>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    this->gnames = GNames(gnames_ptr);
}

void BL2Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x81\xEC\x9C\x0C",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    this->fname_init_ptr = scan<void*>(this->start, this->size, FNAME_INIT_SIG);
    LOG(MISC, "FNameInit: 0x%p", this->fname_init_ptr);
}

void BL2Hook::fname_init(FName* name, const std::wstring& str, int32_t number) {
    this->fname_init(name, str.c_str(), number);
}
void BL2Hook::fname_init(FName* name, const wchar_t* str, int32_t number) {
    // NOLINTNEXTLINE(modernize-use-using)  - need a typedef for the __thiscall
    typedef void*(__thiscall * fname_init_func)(FName * name, const wchar_t* str, int32_t number,
                                                int32_t find_type, int32_t split_name);

    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str, number, 1, 1);
}

}  // namespace unrealsdk::games
