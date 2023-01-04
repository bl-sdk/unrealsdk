#include "pch.h"

#include "env.h"
#include "logging.h"

namespace unrealsdk::logging {

constexpr auto LOG_FILE_NAME = "unrealsdk.log";

constexpr auto CONSOLE_LOG_CALLBACK_NAME = "console";

constexpr auto DEFAULT_FILE_VERBOSITY = Verbosity::MISC;
constexpr auto DEFAULT_CONSOLE_VERBOSITY = Verbosity::CONSOLE;

// This one's just a pain to use safely since it needs to go into C arrays
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_FAKE_EXECUTABLE_NAME "unrealsdk"

static HANDLE external_console_handle = nullptr;
static Verbosity console_verbosity = DEFAULT_CONSOLE_VERBOSITY;

static const char* log_level_to_name(loguru::Verbosity level);
static loguru::Verbosity name_to_log_level(const char* name);

static Verbosity log_level_from_env_var(env::env_var_key key, Verbosity default_value);

static void log_to_console(void* user_data, const loguru::Message& message);

void init(void) {
    loguru::g_preamble_uptime = false;
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::set_verbosity_to_name_callback(log_level_to_name);
    loguru::set_name_to_verbosity_callback(name_to_log_level);

    console_verbosity = log_level_from_env_var(env::CONSOLE_VERBOSITY, DEFAULT_CONSOLE_VERBOSITY);
    auto file_verbosity = log_level_from_env_var(env::FILE_VERBOSITY, DEFAULT_FILE_VERBOSITY);

    loguru::add_file(LOG_FILE_NAME, loguru::Truncate, file_verbosity);

    // Take max Verbosity and filter it in our callback instead
    loguru::add_callback(CONSOLE_LOG_CALLBACK_NAME, log_to_console, nullptr, loguru::Verbosity_MAX);

    loguru::Options options = {};
    options.verbosity_flag = nullptr;

    // loguru::init expects a proper argc/v so we need to fake one
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    int fake_argc = 1;
    char fake_executable[] = LOG_FAKE_EXECUTABLE_NAME;
    char* fake_argv[] = {static_cast<char*>(fake_executable), nullptr};
    loguru::init(fake_argc, static_cast<char**>(fake_argv), options);
    // NOLINTEND(modernize-avoid-c-arrays)

#ifndef DEBUG
    if (env::defined(env::EXTERNAL_CONSOLE))
#endif
    {
        if (AllocConsole() != 0) {
            external_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            if (external_console_handle == nullptr) {
                LOG(ERROR, "Failed to get handle to external console!");
            }
        } else {
            LOG(ERROR, "Failed to initalize external console!");
        }
    }
}

void cleanup(void) {
    loguru::shutdown();
    if (external_console_handle != nullptr) {
        CloseHandle(external_console_handle);
        external_console_handle = nullptr;
    }
}

void set_console_verbosity(Verbosity level) {
    console_verbosity = level;
}

void set_file_verbosity(Verbosity level) {
    loguru::remove_callback(LOG_FILE_NAME);
    loguru::add_file(LOG_FILE_NAME, loguru::Append, level);
}

/**
 * @brief Callback for loguru, used to name our custom log levels.
 * @note Instance of `verbosity_to_name_t`.
 *
 * @param level The verbosity level to try name.
 * @return Pointer to a string holding the name, or null.
 */
static const char* log_level_to_name(loguru::Verbosity level) {
    switch (level) {
        case Verbosity::CONSOLE:
            return "CON";
        case Verbosity::MISC:
            return "MISC";
        case Verbosity::HOOKS:
            return "HOOK";
        case Verbosity::INTERNAL:
            return "INT";
        default:
            return nullptr;
    }
}

/**
 * @brief Callback for loguru, used to parse our custom log levels.
 * @note Instance of `name_to_verbosity_t`.
 *
 * @param name Pointer to a string holding the name to check.
 * @return The parsed verbosity level, or `Verbosity_INVALID`
 */
static loguru::Verbosity name_to_log_level(const char* name) {
    // NOLINTBEGIN(readability-else-after-return)
    if (strcmp(name, "CON") == 0) {
        return Verbosity::CONSOLE;
    } else if (strcmp(name, "MISC") == 0) {
        return Verbosity::MISC;
    } else if (strcmp(name, "HOOK") == 0) {
        return Verbosity::HOOKS;
    } else if (strcmp(name, "INT") == 0) {
        return Verbosity::INTERNAL;
    } else {
        return loguru::Verbosity_INVALID;
    }
    // NOLINTEND(readability-else-after-return)
}

/**
 * @brief Parses a log level from an enviroment variable.
 * @note Accepts both integers (which are in range) and level names
 *
 * @param key The enviroment variable.
 * @param default_value The default value to use if parsing was unsuccessful.
 * @return The parsed verbosity.
 */
static Verbosity log_level_from_env_var(env::env_var_key key, Verbosity default_value) {
    auto level_int = env::get_numeric<int>(key, Verbosity::INVALID);
    if (Verbosity::MIN <= level_int && level_int <= Verbosity::MAX) {
        return static_cast<Verbosity>(level_int);
    }
    auto level_from_str = loguru::get_verbosity_from_name(env::get(env::CONSOLE_VERBOSITY).c_str());
    return level_from_str == Verbosity::INVALID ? default_value
                                                : static_cast<Verbosity>(level_from_str);
}

/**
 * @brief Callback used to write log messages to console.
 * @note Instance of `log_handler_t`.
 *
 * @param message A struct holding the details of the message to log.
 */
static void log_to_console(void* /*user_data*/, const loguru::Message& message) {
    if (message.verbosity > console_verbosity) {
        return;
    }

    if (external_console_handle != nullptr) {
        std::string full_message = message.preamble;
        full_message += message.indentation;
        full_message += message.prefix;
        full_message += message.message;
        full_message += "\n";
        WriteFile(external_console_handle, full_message.c_str(), full_message.size(), nullptr,
                  nullptr);
    }

    // TODO: log to game console
}
}  // namespace unrealsdk::logging
