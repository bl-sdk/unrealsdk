#ifndef UNREALSDK_H
#define UNREALSDK_H

#include "pch.h"

namespace unrealsdk {

struct PathsList {
    std::filesystem::path exe_path;
    std::filesystem::path dll_path;
};

extern const PathsList& paths;

/**
 * @brief Initalizes the SDK.
 *
 * @param this_module Handle to our dll's module.
 */
void init(HMODULE this_module);

}  // namespace unrealsdk

#endif /* UNREALSDK_H */
