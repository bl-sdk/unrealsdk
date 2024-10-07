#ifndef UNREALSDK_ENV_H
#define UNREALSDK_ENV_H

#include "unrealsdk/pch.h"

namespace unrealsdk::env {
using env_var_key = const char*;

const constexpr env_var_key ENV_FILE = "UNREALSDK_ENV_FILE";
const constexpr env_var_key EXTERNAL_CONSOLE = "UNREALSDK_EXTERNAL_CONSOLE";
const constexpr env_var_key LOG_FILE = "UNREALSDK_LOG_FILE";
const constexpr env_var_key LOG_LEVEL = "UNREALSDK_LOG_LEVEL";
const constexpr env_var_key GAME_OVERRIDE = "UNREALSDK_GAME_OVERRIDE";
const constexpr env_var_key UPROPERTY_SIZE = "UNREALSDK_UPROPERTY_SIZE";
const constexpr env_var_key ALLOC_ALIGNMENT = "UNREALSDK_ALLOC_ALIGNMENT";
const constexpr env_var_key CONSOLE_KEY = "UNREALSDK_CONSOLE_KEY";
const constexpr env_var_key UCONSOLE_CONSOLE_COMMAND_VF_INDEX =
    "UNREALSDK_CONSOLE_COMMAND_TEXT_VF_INDEX";
const constexpr env_var_key UCONSOLE_OUTPUT_TEXT_VF_INDEX =
    "UNREALSDK_UCONSOLE_OUTPUT_TEXT_VF_INDEX";
const constexpr env_var_key TREFERENCE_CONTROLLER_DESTROY_OBJ_VF_INDEX =
    "UNREALSDK_TREFERENCE_CONTROLLER_DESTROY_OBJ_VF_INDEX";
const constexpr env_var_key TREFERENCE_CONTROLLER_DESTRUCTOR_VF_INDEX =
    "UNREALSDK_TREFERENCE_CONTROLLER_DESTRUCTOR_VF_INDEX";
const constexpr env_var_key FTEXT_GET_DISPLAY_STRING_VF_INDEX =
    "UNREALSDK_FTEXT_GET_DISPLAY_STRING_VF_INDEX";
const constexpr env_var_key LOCKING_PROCESS_EVENT = "UNREALSDK_LOCKING_PROCESS_EVENT";
const constexpr env_var_key LOG_ALL_CALLS_FILE = "UNREALSDK_LOG_ALL_CALLS_FILE";

namespace defaults {

// Choosing to only define the defaults which are not default constructable

const constexpr auto ENV_FILE = "unrealsdk.env";
const constexpr auto LOG_FILE = "unrealsdk.log";
// EXTERNAL_CONSOLE - defaults to empty string (only used in defined checks)
// LOG_LEVEL - defaults to empty string
// GAME_OVERRIDE - defaults to executable filename - not constant so treat it as an empty string
// UPROPERTY_SIZE - defaults to 0 (meaning auto)
// ALLOC_ALIGNMENT - defaults to 0 (meaning auto)
const constexpr auto CONSOLE_KEY = "Tilde";
const constexpr auto UCONSOLE_CONSOLE_COMMAND_VF_INDEX = 81;
const constexpr auto UCONSOLE_OUTPUT_TEXT_VF_INDEX = 83;
const constexpr auto TREFERENCE_CONTROLLER_DESTROY_OBJ_VF_INDEX = 0;
const constexpr auto TREFERENCE_CONTROLLER_DESTRUCTOR_VF_INDEX = 1;
const constexpr auto FTEXT_GET_DISPLAY_STRING_VF_INDEX = 2;
// LOCKING_PROCESS_EVENT - defaults to empty string (only used in defined checks)
const constexpr env_var_key LOG_ALL_CALLS_FILE = "unrealsdk.calls.tsv";

}  // namespace defaults

#ifndef UNREALSDK_IMPORTING

/**
 * @brief Loads the env var file.
 */
void load_file(void);

#endif

/**
 * @brief Checks if an environment variable is defined.
 *
 * @param env_var The environment variable.
 * @return True if the value is defined, false otherwise.
 */
bool defined(env_var_key env_var);

/**
 * @brief Gets the value of an environment variable.
 *
 * @param envvar The environment variable.
 * @param default_value The default value to return if not defined.
 * @return The environment variable's value.
 */
std::string get(env_var_key env_var, std::string_view default_value = "");

/**
 * @brief Gets the value of an environment variable as a number.
 *
 * @tparam T The number type to parse.
 * @param env_var The environment variable.
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

#endif /* UNREALSDK_ENV_H */
