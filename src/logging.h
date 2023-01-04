#ifndef LOGGING_H
#define LOGGING_H

#include <loguru.hpp>

namespace unrealsdk::logging {

/**
 * @brief The verbosity levels we use in this project
 */
enum Verbosity : loguru::Verbosity {
    /// Log level for errors
    ERROR = loguru::Verbosity_ERROR,
    /// Log level for warnings
    WARNING = loguru::Verbosity_WARNING,
    /// Standard log level for info messages
    INFO = loguru::Verbosity_INFO,

    // Loguru lets us use 1-9 as extra debugging levels.
    // Deliberately only going to use even values so Python can use the levels inbetween

    /// Catch all log level for messages in console - i.e. Python stdout, user input, etc.
    CONSOLE = 2,
    /// Catch all log level for misc info which we want to log, but only to the file
    MISC = 4,
    /// Log level for messages about hooked functions
    HOOKS = 6,
    /// Lowest log level internal debug logging
    INTERNAL = 8,

    /// Minimum valid log level - should only be used for (inclusive) range checks
    MIN = ERROR,
    /// Maximum valid log level - should only be used for (inclusive) range checks
    MAX = loguru::Verbosity_MAX,
    /// Placeholder log level for invalid values
    INVALID = loguru::Verbosity_INVALID,
};

/**
 * @brief Initalize the logging module.
 */
void init(void);

/**
 * @brief Cleanup the logging module.
 */
void cleanup(void);

/**
 * @brief Set the verbosity level of console.
 *
 * @param level The new verbosity level.
 */
void set_console_verbosity(Verbosity level);

/**
 * @brief Set the verbosity level of the log file.
 *
 * @param verbosity The new verbosity level.
 */
void set_file_verbosity(Verbosity level);

}  // namespace unrealsdk::logging

// Can't use LOG_F with our custom levels, so a shortcut for most cases

/**
 * @brief Logs a message.
 *
 * @param level The log level name.
 * @param msg The format string.
 * @param varags Format string contents.
 */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG(level, ...) VLOG_F(unrealsdk::logging::Verbosity::level, __VA_ARGS__)

#endif
