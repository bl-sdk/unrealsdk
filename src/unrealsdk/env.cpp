#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"

namespace unrealsdk::env {

#ifndef UNREALSDK_IMPORTING

void load_file(void) {
    std::ifstream stream{get(ENV_FILE, defaults::ENV_FILE)};

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

std::string get(env_var_key env_var, const std::string& default_value) {
    auto size = GetEnvironmentVariableA(env_var, nullptr, 0);
    if (size == 0) {
        return default_value;
    }

    // NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
    auto buf = reinterpret_cast<char*>(malloc(size * sizeof(char)));
    if (buf == nullptr) {
        return default_value;
    }

    if (GetEnvironmentVariableA(env_var, buf, size) == 0) {
        free(buf);
        return default_value;
    }

    // Size includes the null terminator
    std::string ret{buf, size - 1};

    free(buf);
    // NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

    return ret;
}

}  // namespace unrealsdk::env
