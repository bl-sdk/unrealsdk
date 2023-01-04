#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#ifdef __cplusplus
#include <array>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

// This is the only sdk header we'll include in the PCH, since automatically having the log macro
// exposed is very convenient, and it doesn't depend on any other sdk headers
#include "logging.h"

using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::int8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
#endif

#endif /* PCH_H */
