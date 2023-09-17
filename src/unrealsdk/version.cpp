#include "unrealsdk/pch.h"
#include "unrealsdk/version.h"

namespace unrealsdk {

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(uint32_t, get_version);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(uint32_t, get_version) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    return (VERSION_MAJOR & 0xFF) << 16 | (VERSION_MINOR & 0xFF) << 8 | (VERSION_PATCH & 0xFF);
}
#endif
uint32_t get_version(void) {
    return UNREALSDK_MANGLE(get_version)();
}

namespace {
#ifndef UNREALSDK_IMPORTING

#include "unrealsdk/git.inl"

const constexpr auto GIT_HASH_CHARS = 8;
const std::string VERSION_STR =
    unrealsdk::fmt::format("unrealsdk v{}.{}.{} ({}{})",
                           VERSION_MAJOR,
                           VERSION_MINOR,
                           VERSION_PATCH,
                           std::string(GIT_HEAD_SHA1).substr(0, GIT_HASH_CHARS),
                           GIT_IS_DIRTY ? ", dirty" : "");
#endif
}  // namespace

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI([[nodiscard]] const char*, get_version_str);
#endif
#ifdef UNREALSDK_IMPORTING
const std::string& get_version_string(void) {
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const std::string VERSION_STR = UNREALSDK_MANGLE(get_version_str)();
    return VERSION_STR;
}
#else
const std::string& get_version_string(void) {
    return VERSION_STR;
}
#endif
#ifdef UNREALSDK_EXPORTING
UNREALSDK_CAPI([[nodiscard]] const char*, get_version_str) {
    return VERSION_STR.data();
}
#endif

}  // namespace unrealsdk
