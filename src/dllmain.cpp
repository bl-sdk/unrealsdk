#include "pch.h"

#include "game/selector.h"
#include "unrealsdk.h"

static HMODULE this_module;

/**
 * @brief Main startup thread.
 * @note Instance of `LPTHREAD_START_ROUTINE`.
 *
 * @return unused.
 */
static DWORD WINAPI startup_thread(LPVOID /*unused*/)  {
    try {
        unrealsdk::init(unrealsdk::game::select_based_on_executable());
    } catch (std::exception& ex) {
        LOG(ERROR, "Exception occured while initalizing the sdk: %s", ex.what());
    }

    return 1;
}

/**
 * @brief Main entry point.
 *
 * @param h_module Handle to module for this dll.
 * @param ul_reason_for_call Reason this is being called.
 * @return True if loaded successfully, false otherwise.
 */
// NOLINTNEXTLINE(readability-identifier-naming)  - for `DllMain`
BOOL APIENTRY DllMain(HMODULE h_module, DWORD ul_reason_for_call, LPVOID /*unused*/) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            this_module = h_module;
            DisableThreadLibraryCalls(h_module);
            CreateThread(nullptr, 0, &startup_thread, nullptr, 0, nullptr);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
