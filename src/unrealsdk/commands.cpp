#include "unrealsdk/pch.h"
#include "unrealsdk/commands.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::commands {

namespace {

#ifndef UNREALSDK_IMPORTING

std::unordered_map<std::wstring, AbstractSafeCallback*> commands{};

#endif

}  // namespace

// Use the empty string for the next line token - nothing else should ever be registering it
const std::wstring NEXT_LINE{};

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool, add_command, const wchar_t* cmd, size_t size, AbstractSafeCallback* callback);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool, add_command, const wchar_t* cmd, size_t size, AbstractSafeCallback* callback) {
    std::wstring lower_cmd{cmd, size};
    std::transform(lower_cmd.begin(), lower_cmd.end(), lower_cmd.begin(), &std::towlower);

    if (commands.contains(lower_cmd)) {
        return false;
    }

    commands[std::move(lower_cmd)] = callback;
    return true;
}
#endif

bool add_command(const std::wstring& cmd, const Callback& callback) {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    return UNREALSDK_MANGLE(add_command)(cmd.c_str(), cmd.size(), new SafeCallback(callback));
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool, has_command, const wchar_t* func, size_t size);
#endif
#ifdef UNREALSDK_IMPORTING
bool has_command(const std::wstring& cmd) {
    return UNREALSDK_MANGLE(has_command)(cmd.c_str(), cmd.size());
}
#else
bool has_command(const std::wstring& cmd) {
    return commands.contains(cmd);
}
#endif
#ifdef UNREALSDK_EXPORTING
UNREALSDK_CAPI(bool, has_command, const wchar_t* func, size_t size) {
    return has_command({func, size});
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool, remove_command, const wchar_t* func, size_t size);
#endif
#ifdef UNREALSDK_IMPORTING
bool remove_command(const std::wstring& cmd) {
    return UNREALSDK_MANGLE(remove_command)(cmd.c_str(), cmd.size());
}
#else
bool remove_command(const std::wstring& cmd) {
    if (!commands.contains(cmd)) {
        return false;
    }

    auto callback = commands[cmd];
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete callback;
    commands.erase(cmd);

    return true;
}
#endif
#ifdef UNREALSDK_EXPORTING
UNREALSDK_CAPI(bool, remove_command, const wchar_t* func, size_t size) {
    return remove_command({func, size});
}
#endif

namespace impl {

#ifndef UNREALSDK_IMPORTING

std::pair<AbstractSafeCallback*, size_t> find_matching_command(const std::wstring& line) {
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

#endif

}  // namespace impl

}  // namespace unrealsdk::commands
