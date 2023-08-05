#ifndef UNREALSDK_LOGGING_H
#define UNREALSDK_LOGGING_H

// Because this file in included in the pch, we can't include the pch here instead of these
#include <chrono>
#include <string>
#include "unrealsdk/format.h"

namespace unrealsdk::logging {

enum class Level : uint8_t {
    ERROR = 5,
    WARNING = 4,
    INFO = 3,
    // Intended for warnings which don't concern users, so shouldn't be shown in the UE console
    // (e.g. deprecation warnings)
    DEV_WARNING = 2,
    MISC = 1,

    DEFAULT_CONSOLE_LEVEL = INFO,

    MIN = MISC,
    MAX = ERROR,

    INVALID = 0,
};

struct LogMessage {
    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)

    const uint64_t unix_time_ms{};
    const Level level{};
    const char* msg{};
    const size_t msg_size{};
    const char* location{};
    const int line{};

    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
};

#ifndef UNREALSDK_IMPORTING
/**
 * @brief Initializes logging, creating the log files and external console as needed.
 * @note Only the first call is used. This means manually calling this before initializing the sdk
 *       overrides the defaults.
 *
 * @param file The file to write logs to.
 * @param callbacks_only If true, disables all output, and only runs the logging callbacks. Means
 *                       the filename arg is ignored.
 */
void init(const std::filesystem::path& file, bool callbacks_only = false);
#endif

/**
 * @brief Logs a message.
 * @note Should generally use the `LOG()` macro over this.
 *
 * @param level The log level.
 * @param msg The message.
 * @param location The location the message was logged from. Expected to be either a path, or a
 *                 colon-namespaced function name.
 * @param line The line number the message was logged from.
 */
void log(Level level, const std::string& msg, const char* location, int line);
void log(Level level, const std::wstring& msg, const char* location, int line);

/**
 * @brief Sets the log level of the unreal console.
 * @note Does not affect the log file or external console, if enabled.
 *
 * @param level The new log level.
 * @return True if console level changed, false if an invalid value was passed in.
 */
bool set_console_level(Level level);

using log_callback = void (*)(const LogMessage*);

/**
 * @brief Adds a callback to be run on each log message.
 *
 * @param callback The callback to add.
 */
void add_callback(log_callback callback);

/**
 * @brief Removes a callback from being run on each log message.
 *
 * @param callback The callback to remove.
 */
void remove_callback(log_callback callback);

}  // namespace unrealsdk::logging

/**
 * @brief Logs a message.
 *
 * @param level The log level name.
 * @param ... The format string + it's contents.
 */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG(level, ...)                                                                       \
    unrealsdk::logging::log((unrealsdk::logging::Level::level),                               \
                            unrealsdk::fmt::format(__VA_ARGS__), (const char*)(__FUNCTION__), \
                            (__LINE__))

#endif /* UNREALSDK_LOGGING_H */
