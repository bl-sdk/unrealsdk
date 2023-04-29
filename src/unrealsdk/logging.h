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
    const char* function{};
    const char* file{};
    const int line{};

    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
};

/**
 * @brief Initializes logging, creating the log files and external console as needed.
 * @note Only the first call is used. This means manually calling this before initializing the sdk
 *       overrides the defaults.
 *
 * @param filename The filename to write logs to.
 * @param callbacks_only If true, disables all output, and only runs the logging callbacks. Means
 *                       the filename arg is ignored.
 */
void init(const std::string& filename, bool callbacks_only = false);

/**
 * @brief Internal function to log a message.
 * @note Shouldn't call directly, use the `LOG()` macro instead.
 *
 * @param time The time this message was constructed at.
 * @param level The log level.
 * @param msg The message.
 * @param function The function the message was logged from.
 * @param file The file the message was logged from.
 * @param line The line number the message was logged from.
 */
void log(std::chrono::system_clock::time_point time,
         Level level,
         const std::string& msg,
         const char* function,
         const char* file,
         int line);
void log(std::chrono::system_clock::time_point time,
         Level level,
         const std::wstring& msg,
         const char* function,
         const char* file,
         int line);
/**
 * @brief Sets the log level of the unreal console.
 * @note Does not affect the log file or external console, if enabled.
 *
 * @param level The new log level.
 */
UNREALSDK_CAPI void set_console_level(Level level);

using log_callback = void (*)(const LogMessage*);

/**
 * @brief Adds a callback to be run on each log message.
 *
 * @param callback The callback to add.
 */
UNREALSDK_CAPI void add_callback(log_callback callback);

/**
 * @brief Removes a callback from being run on each log message.
 *
 * @param callback The callback to remove.
 */
UNREALSDK_CAPI void remove_callback(log_callback callback);

}  // namespace unrealsdk::logging

/**
 * @brief Logs a message.
 *
 * @param level The log level name.
 * @param ... The format string + it's contents.
 */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG(level, ...)                                                                           \
    unrealsdk::logging::log(std::chrono::system_clock::now(), (unrealsdk::logging::Level::level), \
                            unrealsdk::fmt::format(__VA_ARGS__), (const char*)(__FUNCTION__),     \
                            (const char*)"", (__LINE__))

#endif /* UNREALSDK_LOGGING_H */
