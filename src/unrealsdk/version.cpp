#include "unrealsdk/pch.h"

#include "unrealsdk/version.h"

namespace unrealsdk {

const std::string VERSION_STR =
    unrealsdk::fmt::format("v{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

#ifdef UNREALSDK_EXPORTING
uint32_t get_version(void) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    return (VERSION_MAJOR & 0xFF) << 16 | (VERSION_MINOR & 0xFF) << 8 | (VERSION_PATCH & 0xFF);
}
#endif

}  // namespace unrealsdk
