#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::game {

namespace {

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

// These are undocumented values, not in the header, treat as size_t to avoid enum conversion errors
constexpr size_t ThreadHideFromDebugger = 17;
constexpr size_t ProcessDebugObjectHandle = 30;
// NOLINTEND(readability-identifier-naming)

// NOLINTBEGIN(readability-identifier-naming)
NtSetInformationThread_func NtSetInformationThread_ptr;
NTSTATUS NTAPI NtSetInformationThread_hook(HANDLE ThreadHandle,
                                           THREAD_INFORMATION_CLASS ThreadInformationClass,
                                           PVOID ThreadInformation,
                                           ULONG ThreadInformationLength) {
    // NOLINTEND(readability-identifier-naming)
    if (static_cast<size_t>(ThreadInformationClass) == ThreadHideFromDebugger) {
        return STATUS_SUCCESS;
    }

    return NtSetInformationThread_ptr(ThreadHandle, ThreadInformationClass, ThreadInformation,
                                      ThreadInformationLength);
}
static_assert(std::is_same_v<decltype(&NtSetInformationThread_hook), NtSetInformationThread_func>,
              "NtSetInformationThread signature is incorrect");

// NOLINTBEGIN(readability-identifier-naming)
NtQueryInformationProcess_func NtQueryInformationProcess_ptr;
NTSTATUS WINAPI NtQueryInformationProcess_hook(HANDLE ProcessHandle,
                                               PROCESSINFOCLASS ProcessInformationClass,
                                               PVOID ProcessInformation,
                                               ULONG ProcessInformationLength,
                                               PULONG ReturnLength) {
    // NOLINTEND(readability-identifier-naming)
    if (static_cast<size_t>(ProcessInformationClass) == ProcessDebugObjectHandle) {
        return STATUS_PORT_NOT_SET;
    }

    return NtQueryInformationProcess_ptr(ProcessHandle, ProcessInformationClass, ProcessInformation,
                                         ProcessInformationLength, ReturnLength);
}
static_assert(
    std::is_same_v<decltype(&NtQueryInformationProcess_hook), NtQueryInformationProcess_func>,
    "NtQueryInformationProcess signature is incorrect");

}  // namespace

void BL1Hook::hook_antidebug(void) {
    MH_STATUS status = MH_OK;

    LPVOID target = nullptr;
    status = MH_CreateHookApiEx(L"ntdll", "NtSetInformationThread",
                                reinterpret_cast<LPVOID>(NtSetInformationThread_hook),
                                reinterpret_cast<LPVOID*>(&NtSetInformationThread_ptr), &target);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create NtSetInformationThread hook: {:x}",
            static_cast<uint32_t>(status));
    } else {
        status = MH_EnableHook(target);
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable NtSetInformationThread hook: {:x}",
                static_cast<uint32_t>(status));
        }
    }

    status = MH_CreateHookApiEx(L"ntdll", "NtQueryInformationProcess",
                                reinterpret_cast<LPVOID>(NtQueryInformationProcess_hook),
                                reinterpret_cast<LPVOID*>(&NtQueryInformationProcess_ptr), &target);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create NtQueryInformationProcess hook: {:x}",
            static_cast<uint32_t>(status));
    } else {
        status = MH_EnableHook(target);
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable NtQueryInformationProcess hook: {:x}",
                static_cast<uint32_t>(status));
        }
    }
}

}  // namespace unrealsdk::game

#endif
