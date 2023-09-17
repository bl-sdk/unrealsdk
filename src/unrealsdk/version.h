#ifndef UNREALSDK_VERSION_H
#define UNREALSDK_VERSION_H

#include "unrealsdk/pch.h"

namespace unrealsdk {

#ifndef UNREALSDK_IMPORTING

#include "unrealsdk/version.inl"
inline constexpr auto VERSION_MAJOR = PROJECT_VERSION_MAJOR;
inline constexpr auto VERSION_MINOR = PROJECT_VERSION_MINOR;
inline constexpr auto VERSION_PATCH = PROJECT_VERSION_PATCH;

#undef PROJECT_VERSION_MAJOR
#undef PROJECT_VERSION_MINOR
#undef PROJECT_VERSION_PATCH

#endif

#ifdef UNREALSDK_SHARED
/**
 * @brief Gets the version number of the sdk.
 * @note Packs as `(major & 0xFF) << 16 | (minor & 0xFF) << 8 | (patch & 0xFF)`.
 *
 * @return The packed version number the sdk was compiled with.
 */
uint32_t get_version(void);
#endif

/**
 * @brief Get the sdk version as a printable string.
 *
 * @return The sdk version string.
 */
const std::string& get_version_string(void);

}  // namespace unrealsdk

#endif /* UNREALSDK_VERSION_H */
