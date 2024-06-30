#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/logging.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::logging {

namespace {

#ifndef UNREALSDK_IMPORTING
// We push log messages into a queue, to be written by another thread
// This means we need to own the strings - the raw LogMessage is just a reference type for callbacks
struct OwnedLogMessage : public LogMessage {
   private:
    std::string msg_str;
    std::string location_str;

   public:
    OwnedLogMessage(uint64_t unix_time_ms,
                    Level level,
                    const char* msg,
                    size_t msg_size,
                    const char* location,
                    size_t location_size,
                    int line)
        : LogMessage(unix_time_ms, level, nullptr, 0, nullptr, 0, line),
          msg_str{msg, msg_size},
          location_str{location, location_size} {}

    /**
     * @brief Fills in the string pointers and decays back into a raw log message.
     *
     * @return A pointer to this message.
     */
    LogMessage* as_ptr(void) {
        this->msg = this->msg_str.data();
        this->msg_size = this->msg_str.size();
        this->location = this->location_str.data();
        this->location_size = this->location_str.size();
        return this;
    }
};

std::mutex pending_messages_mutex{};
std::queue<OwnedLogMessage> pending_messages{};
std::condition_variable pending_messages_available{};

Level unreal_console_level = Level::DEFAULT_CONSOLE_LEVEL;
HANDLE external_console_handle = nullptr;
std::unique_ptr<std::ostream> log_file_stream;

std::mutex callback_mutex{};
std::vector<log_callback> all_log_callbacks{};

#pragma region Logger Thread

[[noreturn]] void logger_thread(void) {
    SetThreadDescription(GetCurrentThread(), L"unrealsdk logger");

    while (true) {
        std::unique_lock<std::mutex> pending_messages_lock(pending_messages_mutex);
        if (pending_messages.empty()) {
            pending_messages_available.wait(pending_messages_lock,
                                            []() { return !pending_messages.empty(); });
        }

        {
            const std::lock_guard<std::mutex> callback_lock(callback_mutex);
            while (!pending_messages.empty()) {
                auto msg = std::move(pending_messages.front());
                pending_messages.pop();

                pending_messages_lock.unlock();

                auto ptr = msg.as_ptr();
                for (const auto& callback : all_log_callbacks) {
                    callback(ptr);
                }

                pending_messages_lock.lock();
            }
        }
    }
}

#pragma endregion
#endif

#pragma region Conversions

/**
 * @brief Gets the current unix time in milliseconds.
 *
 * @return The unix time milliseconds.
 */
uint64_t unix_ms_now(void) {
    auto time = std::chrono::system_clock::now();
    return std::chrono::round<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

#ifndef UNREALSDK_IMPORTING
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
#endif

#pragma endregion

#pragma region Formatting
#ifndef UNREALSDK_IMPORTING

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
        "{1:>{0}%F %T}Z {3:>{2}}@{5:<{4}d} {7:>{6}}| {8}\n", DATE_WIDTH + sizeof(' ') + TIME_WIDTH,
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
                                  "date", TIME_WIDTH + sizeof('Z'), "time", LOCATION_WIDTH,
                                  "location", LINE_WIDTH, "line", LEVEL_WIDTH, "v");
}

#endif
#pragma endregion

#pragma region Built-in Logger
#ifndef UNREALSDK_IMPORTING

void builtin_logger(const LogMessage* msg) {
    if (msg == nullptr) {
        return;
    }

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

#endif
#pragma endregion

}  // namespace

#pragma region Public Interface Implementations
#ifndef UNREALSDK_IMPORTING
namespace impl {

void enqueue_log_msg(uint64_t unix_time_ms,
                     Level level,
                     const char* msg,
                     size_t msg_size,
                     const char* location,
                     size_t location_size,
                     int line) {
    {
        const std::lock_guard<std::mutex> lock(pending_messages_mutex);
        pending_messages.emplace(unix_time_ms, level, msg, msg_size, location, location_size, line);
    }
    pending_messages_available.notify_all();
}

bool set_console_level(Level level) {
    if (Level::MIN > level || level > Level::MAX) {
        LOG(ERROR, "Log level out of range: {}", (uint8_t)level);
        return false;
    }
    unreal_console_level = level;
    return true;
}

void add_callback(log_callback callback) {
    const std::lock_guard<std::mutex> lock(callback_mutex);

    all_log_callbacks.push_back(callback);
}

void remove_callback(log_callback callback) {
    const std::lock_guard<std::mutex> lock(callback_mutex);

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

    // Start the logger thread first thing
    std::thread(logger_thread).detach();

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
UNREALSDK_CAPI(void,
               enqueue_log_msg,
               uint64_t unix_time_ms,
               Level level,
               const char* msg,
               size_t msg_size,
               const char* location,
               size_t location_size,
               int line);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void,
               enqueue_log_msg,
               uint64_t unix_time_ms,
               Level level,
               const char* msg,
               size_t msg_size,
               const char* location,
               size_t location_size,
               int line) {
    impl::enqueue_log_msg(unix_time_ms, level, msg, msg_size, location, location_size, line);
}
#endif
void log(Level level, std::string_view msg, std::string_view location, int line) {
    auto now = unix_ms_now();
    UNREALSDK_MANGLE(enqueue_log_msg)
    (now, level, msg.data(), msg.size(), location.data(), location.size(), line);
}
void log(Level level, std::wstring_view msg, std::string_view location, int line) {
    auto now = unix_ms_now();
    auto narrow = utils::narrow(msg);
    UNREALSDK_MANGLE(enqueue_log_msg)
    (now, level, narrow.data(), narrow.size(), location.data(), location.size(), line);
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
