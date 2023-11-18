#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::env {

#ifndef UNREALSDK_IMPORTING

void load_file(void) {
    std::ifstream stream{utils::get_this_dll().parent_path() / get(ENV_FILE, defaults::ENV_FILE)};

    std::string line;
    while (std::getline(stream, line)) {
        auto equals = line.find_first_of('=');
        if (equals == std::string::npos) {
            continue;
        }

        auto key = line.substr(0, equals);
        if (defined(key.c_str())) {
            continue;
        }

        auto value = line.substr(equals + 1);
        SetEnvironmentVariableA(key.c_str(), value.c_str());
    }
}

#endif

bool defined(env_var_key env_var) {
    return GetEnvironmentVariableA(env_var, nullptr, 0) != 0;
}

std::string get(env_var_key env_var, std::string_view default_value) {
    auto num_chars = GetEnvironmentVariableA(env_var, nullptr, 0);
    if (num_chars == 0) {
        return std::string{default_value};
    }

    // The returned size on failure includes a null terminator, but on success does not.
    std::string ret(num_chars - 1, '\0');
    if (GetEnvironmentVariableA(env_var, ret.data(), num_chars) != (num_chars - 1)) {
        return std::string{default_value};
    }

    return ret;
}

}  // namespace unrealsdk::env
