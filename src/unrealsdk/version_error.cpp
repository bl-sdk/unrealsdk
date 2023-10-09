#include "unrealsdk/pch.h"

namespace unrealsdk {

// Just noinline isn't enough for MSVC, also need to turn all optimizations off :/
#if defined(_MSC_VER) && !defined(__clang__) && !defined(__MINGW32__)
#pragma optimize("", off)
#endif

void throw_version_error(const char* msg) {
    throw std::runtime_error(msg);
}

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__MINGW32__)
#pragma optimize("", on)
#endif

}  // namespace unrealsdk
