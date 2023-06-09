#ifndef UNREALSDK_PCH_H
#define UNREALSDK_PCH_H

#if defined(UNREALSDK_SHARED) || defined(UNREALSDK_EXPORTING)
#if defined(UNREALSDK_EXPORTING)

// If exporting (which is set by cmake privately if building shared)
#if defined(__clang__) || defined(__MINGW32__)
#define UNREALSDK_CAPI extern "C" [[gnu::dllexport]]
#elif defined(_MSC_VER)
#define UNREALSDK_CAPI extern "C" __declspec(dllexport)
#else
#error Unknown dllexport attribute
#endif

// MSVC needs this to allow exceptions through the c interface
// Since it's standard c++, might as well just add it to everything
#define UNREALSDK_CAPI_SUFFIX noexcept(false)

#else

#define UNREALSDK_IMPORTING

// If shared, but not exporting - i.e. when included by something liking against the shared library
#if defined(__clang__) || defined(__MINGW32__)
#define UNREALSDK_CAPI extern "C" [[gnu::dllimport]]
#elif defined(_MSC_VER)
#define UNREALSDK_CAPI extern "C" __declspec(dllimport)
#else
#error Unknown dllimport attribute
#endif

#define UNREALSDK_CAPI_SUFFIX noexcept(false)

#endif
#else

// If not shared nor exporting, just link statically
#define UNREALSDK_CAPI
// Don't need an exception suffix, let the compiler optimize what it wants
#define UNREALSDK_CAPI_SUFFIX

#endif

#define WIN32_LEAN_AND_MEAN
#define WIN32_NO_STATUS
#define NOGDI
#define NOMINMAX
#include <windows.h>
#include <winternl.h>

#undef WIN32_NO_STATUS
#include <ntstatus.h>

#include <MinHook.h>

#ifdef __cplusplus
#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

// This file is just a forwarder for whichever formatting library is configured, it doesn't define
// anything itself, so is fine to include here
#include "unrealsdk/format.h"

// This file is mostly just here so that the `LOG` macro is automatically available everywhere
// It only includes library headers, so is also ok to include
#include "unrealsdk/logging.h"

using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::int8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

#if __cplusplus > 202002L
using std::float32_t;
using std::float64_t;
#else

// NOLINTBEGIN(readability-magic-numbers)
static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits == 24,
              "float is not ieee 32-bit");
static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53,
              "double is not ieee 64-bit");
// NOLINTEND(readability-magic-numbers)

using float32_t = float;
using float64_t = double;

#endif

#ifdef ARCH_X64
static_assert(sizeof(uintptr_t) == sizeof(uint64_t),
              "Architecture define doesn't align with pointer size");
#else
static_assert(sizeof(uintptr_t) == sizeof(uint32_t),
              "Architecture define doesn't align with pointer size");
#endif

#endif

#if defined(UE4) == defined(UE3)
#error Exactly one UE version must be defined
#endif
#if defined(ARCH_X64) == defined(ARCH_X86)
#error Exactly one architecture must be defined
#endif

#endif /* UNREALSDK_PCH_H */
