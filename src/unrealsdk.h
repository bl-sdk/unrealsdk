#ifndef UNREALSDK_H
#define UNREALSDK_H

#include "pch.h"

#include "games/game_hook.h"

namespace unrealsdk {

struct PathsList {
    std::filesystem::path exe_path;
    std::filesystem::path dll_path;
};

extern const PathsList& paths;
extern const std::unique_ptr<games::GameHook>& game;

/**
 * @brief Initalizes the SDK.
 *
 * @param this_module Handle to our dll's module.
 */
void init(HMODULE this_module);

}  // namespace unrealsdk

#endif /* UNREALSDK_H */
