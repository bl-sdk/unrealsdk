#ifndef UNREALSDK_CONFIG_H
#define UNREALSDK_CONFIG_H

#include "unrealsdk/pch.h"

namespace unrealsdk::config {

#ifndef UNREALSDK_IMPORTING

/**
 * @brief Loads the config file.
 */
void load(void);

#endif

/**
 * @brief Gets values from the config file.
 * @note Does not parse the file again, returns references into a static table in `unrealsdk.dll`.
 *
 * @param path The path to the value to get - e.g. "unrealsdk.outerfield[1].innerfield".
 * @return The returned value, or std::nullopt if not set or the incorrect type.
 */
std::optional<bool> get_bool(std::string_view path);
std::optional<int64_t> get_int(std::string_view path);
std::optional<std::string_view> get_str(std::string_view path);

/**
 * @brief Gets the path of the config file, in case you want to do your own parsing.
 *
 * @return The path to the relevant config file.
 */
std::filesystem::path get_base_config_file_path(void);
std::filesystem::path get_user_config_file_path(void);

}  // namespace unrealsdk::config

#endif /* UNREALSDK_CONFIG_H */
