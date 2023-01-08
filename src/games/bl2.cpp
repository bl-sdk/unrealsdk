#include "pch.h"

#include "games/bl2.h"
#include "games/game_hook.h"
#include "MinHook.h"
#include "sigscan.h"
#include "unreal/wrappers/gobjects.h"

using namespace unrealsdk::sigscan;

namespace unrealsdk::games {

#pragma region AntiDebug

// NOLINTBEGIN(modernize-use-using)  - need a typedef for the WINAPI
typedef NTSTATUS(WINAPI* type_NtSetInformationThread)(HANDLE,
                                                      THREAD_INFORMATION_CLASS,
                                                      PVOID,
                                                      ULONG);
typedef NTSTATUS(
    WINAPI* type_NtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
// NOLINTEND(modernize-use-using)

// NOLINTBEGIN(readability-identifier-naming)
static constexpr auto ThreadHideFromDebugger = static_cast<THREAD_INFORMATION_CLASS>(17);
static constexpr auto ProcessDebugObjectHandle = static_cast<PROCESSINFOCLASS>(30);
// NOLINTEND(readability-identifier-naming)

// NOLINTBEGIN(readability-identifier-naming)
type_NtSetInformationThread original_NtSetInformationThread;
NTSTATUS NTAPI hook_NtSetInformationThread(HANDLE ThreadHandle,
                                           THREAD_INFORMATION_CLASS ThreadInformationClass,
                                           PVOID ThreadInformation,
                                           ULONG ThreadInformationLength) {
    // NOLINTEND(readability-identifier-naming)
    if (ThreadInformationClass == ThreadHideFromDebugger) {
        return STATUS_SUCCESS;
    }

    return original_NtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation,
                                           ThreadInformationLength);
}

// NOLINTBEGIN(readability-identifier-naming)
type_NtQueryInformationProcess original_NtQueryInformationProcess;
NTSTATUS WINAPI hook_NtQueryInformationProcess(HANDLE ProcessHandle,
                                               PROCESSINFOCLASS ProcessInformationClass,
                                               PVOID ProcessInformation,
                                               ULONG ProcessInformationLength,
                                               PULONG ReturnLength) {
    // NOLINTEND(readability-identifier-naming)
    if (ProcessInformationClass == ProcessDebugObjectHandle) {
        return STATUS_PORT_NOT_SET;
    }

    return original_NtQueryInformationProcess(ProcessHandle, ProcessInformationClass,
                                              ProcessInformation, ProcessInformationLength,
                                              ReturnLength);
}

void BL2Hook::hook_antidebug(void) {
    MH_STATUS status = MH_OK;

    LPVOID target = nullptr;
    status = MH_CreateHookApiEx(
        L"ntdll", "NtSetInformationThread", reinterpret_cast<LPVOID>(hook_NtSetInformationThread),
        reinterpret_cast<LPVOID*>(&original_NtSetInformationThread), &target);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create NtSetInformationThread hook: %x", status);
    } else {
        status = MH_EnableHook(target);
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable NtSetInformationThread hook: %x", status);
        }
    }

    status =
        MH_CreateHookApiEx(L"ntdll", "NtQueryInformationProcess",
                           reinterpret_cast<LPVOID>(hook_NtQueryInformationProcess),
                           reinterpret_cast<LPVOID*>(&original_NtQueryInformationProcess), &target);
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

void BL2Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x8B\x40\x08",
                                      "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gobjects_instr = scan(this->start, this->size, GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<unreal::GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);

    this->gobjects = unreal::GObjects(gobjects_ptr);
}

void BL2Hook::find_gnames(void) {
    static const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x83\x3C\x81\x00\x74\x5C",
                                    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gnames_instr = scan(this->start, this->size, GNAMES_SIG);
    auto gnames_ptr = read_offset<unreal::GNames::internal_type>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    this->gnames = unreal::GNames(gnames_ptr);
}

void BL2Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x81\xEC\x9C\x0C",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    this->fname_init_ptr = scan<void*>(this->start, this->size, FNAME_INIT_SIG);
    LOG(MISC, "FNameInit: 0x%p", this->fname_init_ptr);
}

void BL2Hook::fname_init(unreal::FName* name, const std::wstring& str, int32_t number) {
    this->fname_init(name, str.c_str(), number);
}
void BL2Hook::fname_init(unreal::FName* name, const wchar_t* str, int32_t number) {
    // NOLINTNEXTLINE(modernize-use-using)  - need a typedef for the __thiscall
    typedef void*(__thiscall * fname_init_func)(unreal::FName * name, const wchar_t* str,
                                                int32_t number, int32_t find_type,
                                                int32_t split_name);

    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str, number, 1, 1);
}

}  // namespace unrealsdk::games
