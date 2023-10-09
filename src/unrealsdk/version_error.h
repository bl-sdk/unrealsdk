#ifndef UNREALSDK_VERSION_ERROR_H
#define UNREALSDK_VERSION_ERROR_H

#include "pch.h"

/*
When attempting to use a feature which is not currently available to the sdk, it may throw a version
error. At this point, these should be caught using simply `std::exception`. A more detailed
exception may be exposed at a later date.
*/

namespace unrealsdk {

/*
Since we sometimes expect version errors to be gated behind ifdefs, previously innocent user code
may sometimes unconditionally throw an error. If the exception gets inlined (including during LTO),
this may cause an unreachable code warning. While we want an unconditional exception, we don't want
the warning, there's nothing wrong with the user's code.

To avoid this, hide throwing behind a noinline helper.
*/

#if defined(__clang__) || defined(__MINGW32__)
#define NOINLINE [[gnu::noinline]]
#elif defined(_MSC_VER)
#define NOINLINE [[msvc::noinline]]
#else
#error Unknown noinline attribute
#endif

/**
 * @brief Helper function to throw a version error.
 * @note This function is (deliberately) not considered noreturn - you may still need to return a
 *       dummy value after calling it.
 *
 * @param msg The exception's message.
 */
NOINLINE void throw_version_error(const char* msg);

#undef NOINLINE

}  // namespace unrealsdk

#endif /* UNREALSDK_VERSION_ERROR_H */
