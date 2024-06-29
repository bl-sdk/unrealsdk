#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/logging.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::logging {

namespace {

#ifndef UNREALSDK_IMPORTING
std::mutex mutex{};

Level unreal_console_level = Level::DEFAULT_CONSOLE_LEVEL;
HANDLE external_console_handle = nullptr;
std::unique_ptr<std::ostream> log_file_stream;

std::vector<log_callback> all_log_callbacks{};
#endif

/**
 * @brief Gets the current unix time in milliseconds.
 *
 * @return The unix time milliseconds.
 */
uint64_t unix_ms_now(void) {
    auto time = std::chrono::system_clock::now();
    return std::chrono::round<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

/**
 * @brief Gets a system clock time point from unix time milliseconds.
 *
 * @param unix_time_ms The unix time milliseconds.
 * @return The system clock time point.
 */
std::chrono::sys_time<std::chrono::milliseconds> time_from_unix_ms(uint64_t unix_time_ms) {
    return std::chrono::round<std::chrono::milliseconds>(
        std::chrono::system_clock::time_point{std::chrono::milliseconds{unix_time_ms}});
}

/**
 * @brief Gets the name of a log level.
 *
 * @param level The log level
 * @return The level's name.
 */
std::string get_level_name(Level level) {
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

/**
 * @brief Gets a log level from it's string representation.
 *
 * @param str The string.
 * @return The parsed log level, or `Level::INVALID`.
 */
Level get_level_from_string(std::string_view str) {
    if (str.empty()) {
        return Level::INVALID;
    }

    // Start by matching first character
    switch (std::toupper(str[0])) {
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
    auto res = std::from_chars(str.data(), str.data() + str.size(), int_level);
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

#pragma region Formatting

const std::string TRUNCATION_PREFIX = "~ ";

/**
 * @brief Truncates leading chunks of a string until it fits under a max width.
 * @note Will return strings longer than the max width if it can't cleanly chunk them.
 *
 * @param str The string to truncate.
 * @param separators The characters to use as separators between chunks.
 * @param max_width The maximum width of the string.
 * @return The truncated string.
 */
std::string truncate_leading_chunks(const std::string&& str,
                                    std::string_view separators,
                                    size_t max_width) {
    auto width = str.size();
    size_t start_pos = 0;
    while (width > max_width) {
        auto next_separator_char = str.find_first_of(separators, start_pos);
        if (next_separator_char == std::string::npos) {
            break;
        }
        auto next_regular_char = str.find_first_not_of(separators, next_separator_char);
        if (next_regular_char == std::string::npos) {
            break;
        }

        // The first time we truncate something, we know we now need to add the prefix on, so
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

constexpr auto DATE_WIDTH = 10;
constexpr auto TIME_WIDTH = 12;
constexpr auto LOCATION_WIDTH = 50;
constexpr auto LINE_WIDTH = 4;
constexpr auto LEVEL_WIDTH = 4;

/**
 * @brief Formats a log message following our internal style.
 *
 * @param msg The log message.
 * @return The formatted message
 */
std::string format_message(const LogMessage& msg) {
    return unrealsdk::fmt::format(
        "{1:>{0}%F %T}Z {3:>{2}}@{5:<{4}d} {7:>{6}}| {8}\n", DATE_WIDTH + 1 + TIME_WIDTH + 1,
        time_from_unix_ms(msg.unix_time_ms), LOCATION_WIDTH,
        truncate_leading_chunks(msg.location, "\\/:", LOCATION_WIDTH), LINE_WIDTH, msg.line,
        LEVEL_WIDTH, get_level_name(msg.level), std::string{msg.msg, msg.msg_size});
}

/**
 * @brief Gets a header to display at the top of the log file
 *
 * @return The header.
 */
std::string get_header(void) {
    return unrealsdk::fmt::format("{1:<{0}} {3:<{2}} {5:>{4}}@{7:<{6}} {9:>{8}}| \n", DATE_WIDTH,
                                  "date", TIME_WIDTH + 1, "time", LOCATION_WIDTH, "location",
                                  LINE_WIDTH, "line", LEVEL_WIDTH, "v");
}

#pragma endregion

#pragma region Built-in Logger

void builtin_logger(const LogMessage* msg) {
    if (unreal_console_level != Level::INVALID && unreal_console_level <= msg->level) {
        unrealsdk::uconsole_output_text(utils::widen({msg->msg, msg->msg_size}));
    }

    if (external_console_handle != nullptr || log_file_stream) {
        auto formatted = format_message(*msg);

        if (external_console_handle != nullptr) {
            WriteFile(external_console_handle, formatted.c_str(), (DWORD)formatted.size(), nullptr,
                      nullptr);
        }

        if (log_file_stream) {
            *log_file_stream << formatted << std::flush;
        }
    }
}

#pragma endregion

}  // namespace

#pragma region Public Interface Implementations
#ifndef UNREALSDK_IMPORTING
namespace impl {

void run_log_callbacks(const LogMessage* msg) {
    if (msg == nullptr) {
        return;
    }

    const std::lock_guard<std::mutex> lock(mutex);

    for (const auto& callback : all_log_callbacks) {
        callback(msg);
    }
};

bool set_console_level(Level level) {
    if (Level::MIN > level || level > Level::MAX) {
        LOG(ERROR, "Log level out of range: {}", (uint8_t)level);
        return false;
    }
    unreal_console_level = level;
    return true;
}

void add_callback(log_callback callback) {
    const std::lock_guard<std::mutex> lock(mutex);

    all_log_callbacks.push_back(callback);
}

void remove_callback(log_callback callback) {
    const std::lock_guard<std::mutex> lock(mutex);

    all_log_callbacks.erase(
        std::remove(all_log_callbacks.begin(), all_log_callbacks.end(), callback),
        all_log_callbacks.end());
}

}  // namespace impl

void init(const std::filesystem::path& file, bool unreal_console) {
    static bool initialized = false;
    if (initialized) {
        return;
    }
    initialized = true;

    if (unreal_console) {
        auto env_level = get_level_from_string(env::get(env::LOG_LEVEL));
        if (env_level != Level::INVALID) {
            unreal_console_level = env_level;
        }
    } else {
        unreal_console_level = Level::INVALID;
    }

    if (!file.empty()) {
        log_file_stream = std::make_unique<std::ofstream>(file, std::ofstream::trunc);
        *log_file_stream << get_header() << std::flush;
    }

    // Add the builtin logger now, after initializing the above two streams, so that the external
    // console error messages actually have somewhere to go
    impl::add_callback(&builtin_logger);

#ifdef NDEBUG
    if (env::defined(env::EXTERNAL_CONSOLE))
#endif
    {
        if (AllocConsole() != 0) {
            external_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            if (external_console_handle == nullptr) {
                LOG(ERROR, "Failed to get handle to external console!");
            }
        } else {
            LOG(ERROR, "Failed to initialize external console!");
        }
    }
}
#endif
#pragma endregion

// =================================================================================================

#pragma region C API Wrappers

#ifdef UNREALSDK_SHARED
// Keeping the old symbol name, even though we've since renamed the implementation
UNREALSDK_CAPI(void, log_msg_internal, const LogMessage* msg);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, log_msg_internal, const LogMessage* msg) {
    impl::run_log_callbacks(msg);
}
#endif
void log(Level level, std::string_view msg, const char* location, int line) {
    const LogMessage log_msg{unix_ms_now(), level, msg.data(), msg.size(), location, line};
    UNREALSDK_MANGLE(log_msg_internal)(&log_msg);
}
void log(Level level, std::wstring_view msg, const char* location, int line) {
    auto narrow = utils::narrow(msg);
    const LogMessage log_msg{unix_ms_now(), level, narrow.data(), narrow.size(), location, line};
    UNREALSDK_MANGLE(log_msg_internal)(&log_msg);
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool, set_console_level, Level level);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool, set_console_level, Level level) {
    return impl::set_console_level(level);
}
#endif
bool set_console_level(Level level) {
    return UNREALSDK_MANGLE(set_console_level)(level);
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(void, add_callback, log_callback callback);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, add_callback, log_callback callback) {
    impl::add_callback(callback);
}
#endif
void add_callback(log_callback callback) {
    UNREALSDK_MANGLE(add_callback)(callback);
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(void, remove_callback, log_callback callback);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, remove_callback, log_callback callback) {
    impl::remove_callback(callback);
}
#endif
void remove_callback(log_callback callback) {
    UNREALSDK_MANGLE(remove_callback)(callback);
}

#pragma endregion

}  // namespace unrealsdk::logging
