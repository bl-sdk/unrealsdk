#ifndef UNREALSDK_EXPORTS_H
#define UNREALSDK_EXPORTS_H

/*
Helper macros to deal with exported functions.

User code should never need to use these, they should always be hidden in implementation files.
*/

/**
 * @fn UNREALSDK_MANGLE
 * @brief Mangles an exported symbol name.
 *
 * @param name The name to mangle.
 * @return The mangled name
 */

/**
 * @fn UNREALSDK_CAPI
 * @brief Declares an exported C function.
 * @note The exported function is automatically mangled, use `UNREALSDK_MANGLE()` when calling it.
 *
 * @param ret The return type. May include attributes.
 * @param name The function name.
 * @param ... The function args (including types).
 * @returns A function declaration. May be left as a forward declaration, or used in the definition.
 */

// =================================================================================================

// Unconditionally mangle - if we disabled this when not shared we might cause name conflicts (e.g.
// if only the return type differs)
#define UNREALSDK_MANGLE(name) _unrealsdk_export__##name

#if defined(UNREALSDK_SHARED) || defined(UNREALSDK_EXPORTING)
// If compiled as shared (regardless of which side we're compiling for)

// Determine the correct dllimport/export attribute
#if defined(UNREALSDK_EXPORTING)

#if defined(__clang__) || defined(__MINGW32__)
#define UNREALSDK_DLLEXPORT [[gnu::dllexport]]
#elif defined(_MSC_VER)
#define UNREALSDK_DLLEXPORT __declspec(dllexport)
#else
#error Unknown dllexport attribute
#endif

#else  //  defined(UNREALSDK_EXPORTING)

#if defined(__clang__) || defined(__MINGW32__)
#define UNREALSDK_DLLEXPORT [[gnu::dllimport]]
#elif defined(_MSC_VER)
#define UNREALSDK_DLLEXPORT __declspec(dllimport)
#else
#error Unknown dllimport attribute
#endif

#endif  //  defined(UNREALSDK_EXPORTING)

// Need extern C to create a valid export
// Use the relevant dllimport/export attribute
// Mangle the function name to avoid conflicts
// MSVC needs noexpect(false) to allow exceptions through the C interface - since it's standard C++,
//  might as well just add across all compilers
#define UNREALSDK_CAPI(ret, name, ...) \
    extern "C" UNREALSDK_DLLEXPORT ret UNREALSDK_MANGLE(name)(__VA_ARGS__) noexcept(false)

// For convenience, define an importing flag
#if !defined(UNREALSDK_EXPORTING)
#define UNREALSDK_IMPORTING
#endif

#else  // defined(UNREALSDK_SHARED) || defined(UNREALSDK_EXPORTING)
// If statically linked

// Just do a basic function declaration
#define UNREALSDK_CAPI(ret, name, ...) ret UNREALSDK_MANGLE(name)(__VA_ARGS__)

#endif  // defined(UNREALSDK_SHARED) || defined(UNREALSDK_EXPORTING)

#endif /* UNREALSDK_EXPORTS_H */
