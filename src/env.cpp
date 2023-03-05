#include "pch.h"

#include "env.h"

namespace unrealsdk::env {

#ifdef __clang__
// clang considers getenv deprecated
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

bool defined(env_var_key env_var) {
    return std::getenv(env_var) != nullptr;
}

std::string get(env_var_key env_var, const std::string& default_value) {
    const char* buf = std::getenv(env_var);
    if (buf == nullptr) {
        return default_value;
    }

    return std::string{buf};
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

}  // namespace unrealsdk::env
