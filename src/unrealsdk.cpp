#include "pch.h"

#include "game/game_hook.h"
#include "hook_manager.h"
#include "unrealsdk.h"

namespace unrealsdk {

// Keep an internal mutable var, but only expose a const reference
static PathsList paths_internal;
const PathsList& paths = paths_internal;

hook_manager::map hooks{};
bool log_all_calls = false;

/**
 * @brief Updates the paths lists.
 *
 * @param this_module Handle to our dll's module.
 */
static void update_paths(HMODULE this_module) {
    std::array<char, FILENAME_MAX> buf{};

    if (GetModuleFileNameA(nullptr, buf.data(), buf.size()) > 0) {
        paths_internal.exe_path = std::filesystem::path(buf.data());
    } else {
        paths_internal.exe_path = "";
    }

    if (GetModuleFileNameA(this_module, buf.data(), buf.size()) > 0) {
        paths_internal.dll_path = std::filesystem::path(buf.data());
    } else {
        paths_internal.dll_path = "";
    }
}

void init(HMODULE this_module) {
    LOG(INFO, "Initalizing");

    if (MH_Initialize() != MH_OK) {
        throw std::runtime_error("Minhook initialization failed!");
    }

    update_paths(this_module);

    game::init();
}

}  // namespace unrealsdk
