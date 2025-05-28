#include "unrealsdk/pch.h"
#include "unrealsdk/commands.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::commands {

namespace {

#ifndef UNREALSDK_IMPORTING

utils::StringViewMap<std::wstring, DLLSafeCallback*> commands{};

#endif

}  // namespace

// Use the empty string for the next line token - nothing else should ever be registering it
const std::wstring NEXT_LINE{};

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool, add_command, const wchar_t* cmd, size_t size, DLLSafeCallback* callback);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool, add_command, const wchar_t* cmd, size_t size, DLLSafeCallback* callback) {
    std::wstring lower_cmd(size, '\0');
    std::transform(cmd, cmd + size, lower_cmd.begin(), &std::towlower);

    if (commands.contains(lower_cmd)) {
        return false;
    }

    commands.emplace(std::move(lower_cmd), callback);
    return true;
}
#endif

bool add_command(std::wstring_view cmd, const Callback& callback) {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    return UNREALSDK_MANGLE(add_command)(cmd.data(), cmd.size(), new DLLSafeCallback(callback));
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool, has_command, const wchar_t* cmd, size_t size);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool, has_command, const wchar_t* cmd, size_t size) {
    std::wstring lower_cmd(size, '\0');
    std::transform(cmd, cmd + size, lower_cmd.begin(), &std::towlower);
    return commands.contains(lower_cmd);
}
#endif

bool has_command(std::wstring_view cmd) {
    return UNREALSDK_MANGLE(has_command)(cmd.data(), cmd.size());
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool, remove_command, const wchar_t* cmd, size_t size);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool, remove_command, const wchar_t* cmd, size_t size) {
    std::wstring lower_cmd(size, '\0');
    std::transform(cmd, cmd + size, lower_cmd.begin(), &std::towlower);
    auto iter = commands.find(lower_cmd);

    if (iter == commands.end()) {
        return false;
    }

    iter->second->destroy();
    commands.erase(iter);

    return true;
}
#endif

bool remove_command(std::wstring_view cmd) {
    return UNREALSDK_MANGLE(remove_command)(cmd.data(), cmd.size());
}

namespace impl {

#ifndef UNREALSDK_IMPORTING

bool is_command_valid(std::wstring_view line, bool direct_user_input) {
    if (direct_user_input && commands.find(NEXT_LINE) != commands.end()) {
        return true;
    }
    auto non_space = std::ranges::find_if_not(line, &std::iswspace);
    if (non_space == line.end()) {
        return false;
    }

    auto cmd_end = std::find_if(non_space, line.end(), &std::iswspace);

    std::wstring cmd(cmd_end - non_space, '\0');
    std::transform(non_space, cmd_end, cmd.begin(), &std::towlower);
    return commands.contains(cmd);
}

void run_command(std::wstring_view line) {
    auto iter = commands.find(NEXT_LINE);
    if (iter != commands.end()) {
        auto callback = iter->second;
        commands.erase(iter);

        callback->operator()(line.data(), line.size(), 0);

        callback->destroy();
        return;
    }

    // I realize we're redoing a bunch of work from above here, but meh.
    // Hopefully LTO gets it
    auto non_space = std::ranges::find_if_not(line, &std::iswspace);
    if (non_space == line.end()) {
        return;
    }

    auto cmd_end = std::find_if(non_space, line.end(), &std::iswspace);

    std::wstring cmd(cmd_end - non_space, '\0');
    std::transform(non_space, cmd_end, cmd.begin(), &std::towlower);

    commands.at(cmd)->operator()(line.data(), line.size(), cmd_end - line.begin());
}

#endif

}  // namespace impl

}  // namespace unrealsdk::commands
