#include "pch.h"

/**
 * @brief Main startup thread.
 * @note Instance of `LPTHREAD_START_ROUTINE`.
 *
 * @return unused.
 */
static DWORD startup_thread(void* /*unused*/) {
    try {
        SetThreadDescription(GetCurrentThread(), L"UnrealSDK Startup");
    } catch (std::exception& ex) {}

    return 1;
}

/**
 * @brief Main entry point.
 *
 * @param hModule Handle to module for this dll.
 * @param ul_reason_for_call Reason this is being called.
 * @return True if loaded successfully, false otherwise.
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*unused*/) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, &startup_thread, nullptr, 0, nullptr);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
