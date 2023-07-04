#include "unrealsdk/pch.h"
#include "unrealsdk/commands.h"

namespace unrealsdk::commands {

namespace {

std::unordered_map<std::wstring, Callback*> commands{};

}  // namespace

// Use the empty string for the next line token - nothing else should ever be registering it
const std::wstring NEXT_LINE{};

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI bool add_command(const wchar_t* cmd,
                                size_t size,
                                Callback* callback) UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
bool add_command(const std::wstring& cmd, Callback* callback) {
    return add_command(cmd.c_str(), cmd.size(), callback);
}
#else
bool add_command(const std::wstring& cmd, Callback* callback) {
    std::wstring lower_cmd = cmd;
    std::transform(lower_cmd.begin(), lower_cmd.end(), lower_cmd.begin(), &std::towlower);

    if (commands.contains(lower_cmd)) {
        return false;
    }

    commands[std::move(lower_cmd)] = callback;
    return true;
}
#endif
#ifdef UNREALSDK_EXPORTING
bool add_command(const wchar_t* cmd, size_t size, Callback* callback) {
    return add_command({cmd, size}, callback);
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI bool has_command(const wchar_t* func, size_t size) UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
bool has_command(const std::wstring& cmd) {
    return has_command(cmd.c_str(), cmd.size());
}
#else
bool has_command(const std::wstring& cmd) {
    return commands.contains(cmd);
}
#endif
#ifdef UNREALSDK_EXPORTING
bool has_command(const wchar_t* func, size_t size) {
    return has_command({func, size});
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI bool remove_command(const wchar_t* func, size_t size) UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
bool remove_command(const std::wstring& cmd) {
    return remove_command(cmd.c_str(), cmd.size());
}
#else
bool remove_command(const std::wstring& cmd) {
    return commands.erase(cmd) > 0;
}
#endif
#ifdef UNREALSDK_EXPORTING
bool remove_command(const wchar_t* func, size_t size) {
    return remove_command({func, size});
}
#endif

namespace impl {

std::pair<Callback*, size_t> find_matching_command(const std::wstring& line) {
    if (commands.contains(NEXT_LINE)) {
        auto callback = commands[NEXT_LINE];
        commands.erase(NEXT_LINE);
        return {callback, 0};
    }

    auto non_space = std::find_if_not(line.begin(), line.end(), &std::iswspace);
    if (non_space == line.end()) {
        return {nullptr, 0};
    }

    auto cmd_end = std::find_if(non_space, line.end(), &std::iswspace);

    std::wstring cmd{non_space, cmd_end};
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), &std::towlower);

    return commands.contains(cmd) ? std::pair{commands[cmd], cmd_end - line.begin()}
                                  : std::pair{nullptr, 0};
}

}  // namespace impl

}  // namespace unrealsdk::commands
