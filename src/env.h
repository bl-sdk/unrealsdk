#ifndef ENV_H
#define ENV_H

namespace unrealsdk::env {
using env_var_key = const char*;

constexpr env_var_key EXTERNAL_CONSOLE = "UNREALSDK_EXTERNAL_CONSOLE";
constexpr env_var_key CONSOLE_VERBOSITY = "UNREALSDK_CONSOLE_LOG_LEVEL";
constexpr env_var_key FILE_VERBOSITY = "UNREALSDK_FILE_LOG_LEVEL";
constexpr env_var_key GAME_OVERRIDE = "UNREALSDK_GAME_OVERRIDE";
constexpr env_var_key UPROPERTY_SIZE = "UNREALSDK_UPROPERTY_SIZE";
constexpr env_var_key ALLOC_ALIGNMENT = "UNREALSDK_ALLOC_ALIGNMENT";
constexpr env_var_key CONSOLE_KEY = "UNREALSDK_CONSOLE_KEY";
constexpr env_var_key UCONSOLE_OUTPUT_TEXT_VF_INDEX = "UNREALSDK_UCONSOLE_OUTPUT_TEXT_VF_INDEX";

constexpr auto CONSOLE_KEY_DEFAULT = "Tilde";

/**
 * @brief Checks if an enviroment variable is defined.
 *
 * @param env_var The enviroment variable.
 * @return True if the value is defined, false otherwise.
 */
bool defined(env_var_key env_var);

/**
 * @brief Gets the value of an enviroment variable.
 *
 * @param envvar The enviroment variable.
 * @param default_value The default value to return if not defined.
 * @return The enviroment variable's value.
 */
std::string get(env_var_key env_var, const std::string& default_value = "");

/**
 * @brief Gets the value of an enviroment variable as a number.
 *
 * @tparam T The number type to parse.
 * @param env_var The enviroment variable.
 * @param default_value The default value to return, if not defined or if parsing fails.
 * @return The parsed number.
 */
template <typename T>
T get_numeric(env_var_key env_var, T default_value = 0) {
    auto str = get(env_var);

    T value;
    auto res = std::from_chars(str.c_str(), str.c_str() + str.size(), value);
    return res.ec == std::errc() ? value : default_value;
}
}  // namespace unrealsdk::env

#endif /* ENV_H */
