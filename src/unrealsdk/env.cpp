#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"

namespace unrealsdk::env {

// `getenv` is considered deprecated, suppress that warning
// There isn't cross platform alternative that's as easy, and we use it safely, immediately turning
// the buffer into a `std::string`
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace unrealsdk::env
