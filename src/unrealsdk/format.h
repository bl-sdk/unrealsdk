#ifndef UNREALSDK_FORMAT_H
#define UNREALSDK_FORMAT_H

// Header to switch between std::format and fmtlib, and alias them under `unrealsdk::fmt`

// NOLINTBEGIN(misc-unused-using-decls)

#if __cpp_lib_format

#include <format>

namespace unrealsdk::fmt {

using std::format;
using std::format_context;
using std::formatter;

}  // namespace unrealsdk::fmt

#else

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/xchar.h>

namespace unrealsdk::fmt {

using ::fmt::format;
using ::fmt::format_context;
using ::fmt::formatter;

}  // namespace unrealsdk::fmt

#endif

// NOLINTEND(misc-unused-using-decls)

#endif /* UNREALSDK_FORMAT_H */
