#include "pch.h"

#include "env.h"

namespace unrealsdk::env {

// clang considers getenv deprecated
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

bool defined(env_var_key env_var) {
    return std::getenv(env_var) != nullptr;
}

std::string get(env_var_key env_var) {
    const char* buf = std::getenv(env_var);
    if (buf == nullptr) {
        return "";
    }

    return std::string{buf};
}

#pragma clang diagnostic pop

}  // namespace unrealsdk::env
