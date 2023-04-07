#include "unrealsdk/pch.h"

#include "unrealsdk/version.h"

namespace unrealsdk {

const std::string VERSION_STR =
    unrealsdk::fmt::format("v{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

}
