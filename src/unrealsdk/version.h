#ifndef UNREALSDK_VERSION_H
#define UNREALSDK_VERSION_H

#include "unrealsdk/pch.h"

namespace unrealsdk {

inline constexpr auto VERSION_MAJOR = 1;
inline constexpr auto VERSION_MINOR = 0;
inline constexpr auto VERSION_PATCH = 0;

#ifdef UNREALSDK_SHARED
/**
 * @brief Gets the version number of the sdk.
 * @note Packs as `(major & 0xFF) << 16 | (minor & 0xFF) << 8 | (patch & 0xFF)`.
 *
 * @return The packed version number the sdk was compiled with.
 */
UNREALSDK_CAPI uint32_t get_version(void);
#endif

/**
 * @brief Get the sdk version as a printable string.
 *
 * @return The sdk version string.
 */
const std::string& get_version_string(void);

}  // namespace unrealsdk

#endif /* UNREALSDK_VERSION_H */
