#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <ntstatus.h>
#include <windows.h>
#include <winternl.h>

#include "MinHook.h"

#ifdef __cplusplus
#include <array>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

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

static_assert(sizeof(float) == sizeof(uint32_t), "float is not 32-bit");
static_assert(sizeof(double) == sizeof(uint64_t), "double is not 64-bit");
using float32_t = float;
using float64_t = double;
#if __cplusplus >= 202002L
#warning Replace float32/64 with standard versions
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

#endif /* PCH_H */
