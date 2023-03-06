#include "pch.h"

#include "env.h"
#include "logging.h"
#include "unrealsdk.h"
#include "utils.h"

namespace unrealsdk::logging {

static constexpr auto LOG_FILE_NAME = "unrealsdk.log";

static std::mutex mutex{};

static std::atomic<Level> unreal_console_level = Level::DEFAULT_CONSOLE_LEVEL;
static HANDLE external_console_handle = nullptr;
static std::unique_ptr<std::ostream> log_file_stream;

static std::vector<log_callback> all_log_callbacks{};

bool callbacks_only = false;

#pragma region Formatting

/**
 * @brief Truncates leading chunks of a string until it fits under a max width.
 * @note Will return strings longer than the max width if it can't cleanly chunk them.
 *
 * @param str The string to truncate.
 * @param seperators The characters to use as seperators between chunks.
 * @param max_width The maximum width of the string.
 * @return The truncated string.
 */
static std::string truncate_leading_chunks(const std::string& str,
                                           const std::string& seperators,
                                           size_t max_width) {
    static const std::string TRUNCATION_PREFIX = "~ ";

    auto width = str.size();
    size_t start_pos = 0;
    while (width > max_width) {
        auto next_seperator_char = str.find_first_of(seperators, start_pos);
        if (next_seperator_char == std::string::npos) {
            break;
        }
        auto next_regular_char = str.find_first_not_of(seperators, next_seperator_char);
        if (next_regular_char == std::string::npos) {
            break;
        }

        // The first time we truncate something, we know we noew need to add the prefix on, so
        // subtract it from max width
        if (start_pos == 0) {
            max_width -= TRUNCATION_PREFIX.size();
        }

        width -= (next_regular_char - start_pos);
        start_pos = next_regular_char;
    }

    if (start_pos == 0) {
        return str;
    }

    return TRUNCATION_PREFIX + str.substr(start_pos);
}

/**
 * @brief Gets the name of a log level.
 *
 * @param level The log level
 * @return The level's name.
 */
static std::string get_level_name(Level level) {
    switch (level) {
        default:
        case Level::ERROR:
            return "ERR";
        case Level::WARNING:
            return "WARN";
        case Level::INFO:
            return "INFO";
        case Level::DEV_WARNING:
            return "DWRN";
        case Level::MISC:
            return "MISC";
    }
}

static constexpr auto DATE_WIDTH = 10;
static constexpr auto TIME_WIDTH = 12;
static constexpr auto LOCATION_WIDTH = 50;
static constexpr auto LINE_WIDTH = 4;
static constexpr auto LEVEL_WIDTH = 4;

/**
 * @brief Formats a log message following our internal style.
 *
 * @param msg The log message.
 * @return The formatted message
 */
static std::string format_message(const LogMessage& msg) {
    auto location = msg.function[0] != '\0'
                        ? truncate_leading_chunks(msg.function, ":", LOCATION_WIDTH)
                        : truncate_leading_chunks(msg.file, "\\/", LOCATION_WIDTH);

    return std::format(
        "{1:>{0}%F %T} {3:>{2}}@{5:<{4}d} {7:>{6}}| {8}\n", DATE_WIDTH + TIME_WIDTH + 1,
        std::chrono::round<std::chrono::milliseconds>(msg.time), LOCATION_WIDTH, location,
        LINE_WIDTH, msg.line, LEVEL_WIDTH, get_level_name(msg.level), msg.msg);
}

/**
 * @brief Gets a header to display at the top of the log file
 *
 * @return The header.
 */
static std::string get_header(void) {
    return std::format("{1:<{0}} {3:<{2}} {5:>{4}}@{7:<{6}} {9:>{8}}| \n", DATE_WIDTH, "date",
                       TIME_WIDTH, "time", LOCATION_WIDTH, "location", LINE_WIDTH, "line",
                       LEVEL_WIDTH, "v");
}

#pragma endregion

/**
 * @brief Gets a log level from it's string representation.
 *
 * @param str The string.
 * @return The parsed log level, or `Level::INVALID`.
 */
static Level get_level_from_string(const std::string& str) {
    if (str.empty()) {
        return Level::INVALID;
    }

    // Start by matching first character
    switch (str[0]) {
        case 'E':
            return Level::ERROR;
        case 'W':
            return Level::WARNING;
        case 'I':
            return Level::INFO;
        case 'D':
            return Level::DEV_WARNING;
        case 'M':
            return Level::MISC;
        default:
            break;
    }

    // Otherwise try parse as an int
    uint32_t int_level = 0;
    auto res = std::from_chars(str.c_str(), str.c_str() + str.size(), int_level);
    if (res.ec == std::errc()) {
        return Level::INVALID;
    }
    // If within range
    if (static_cast<decltype(int_level)>(Level::MIN) <= int_level
        && int_level <= static_cast<decltype(int_level)>(Level::MAX)) {
        return static_cast<Level>(int_level);
    }

    return Level::INVALID;
}

void init(void) {
    if (callbacks_only) {
        return;
    }

    auto env_level = get_level_from_string(env::get(env::LOG_LEVEL));
    if (env_level != Level::INVALID) {
        unreal_console_level = env_level;
    }

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

    log_file_stream = std::make_unique<std::ofstream>(LOG_FILE_NAME, std::ofstream::trunc);
    *log_file_stream << get_header() << std::flush;
}

void log(const LogMessage&& msg) {
    std::lock_guard<std::mutex> lock(mutex);

    for (const auto& callback : all_log_callbacks) {
        callback(msg);
    }

    if (callbacks_only) {
        return;
    }

    if (unreal_console_level >= msg.level) {
        unrealsdk::uconsole_output_text(utils::widen(msg.msg));
    }

    if (external_console_handle != nullptr || log_file_stream) {
        auto formatted = format_message(msg);

        if (external_console_handle != nullptr) {
            WriteFile(external_console_handle, formatted.c_str(), formatted.size(), nullptr,
                      nullptr);
        }

        if (log_file_stream) {
            *log_file_stream << formatted << std::flush;
        }
    }
}

void set_console_level(Level level) {
    if (Level::MIN > level || level > Level::MAX) {
        throw std::out_of_range("Log level out of range!");
    }
    unreal_console_level = level;
}

void add_callback(log_callback callback) {
    std::lock_guard<std::mutex> lock(mutex);

    all_log_callbacks.push_back(callback);
}

void remove_callback(log_callback callback) {
    std::lock_guard<std::mutex> lock(mutex);

    all_log_callbacks.erase(
        std::remove(all_log_callbacks.begin(), all_log_callbacks.end(), callback),
        all_log_callbacks.end());
}

}  // namespace unrealsdk::logging
