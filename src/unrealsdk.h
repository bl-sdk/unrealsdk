#ifndef UNREALSDK_H
#define UNREALSDK_H

#include "pch.h"

#include "games/game_hook.h"
#include "hook_manager.h"

namespace unrealsdk {

struct PathsList {
    std::filesystem::path exe_path;
    std::filesystem::path dll_path;
};

extern const PathsList& paths;
extern const std::unique_ptr<games::GameHook>& game;
extern hook_manager::map hooks;
extern bool log_all_calls;

/**
 * @brief Initalizes the SDK.
 *
 * @param this_module Handle to our dll's module.
 */
void init(HMODULE this_module);

}  // namespace unrealsdk

#endif /* UNREALSDK_H */
