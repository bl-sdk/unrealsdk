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

    commands.emplace(lower_cmd, callback);
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
    const std::wstring_view view{cmd, size};
    return commands.contains(view);
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
    const std::wstring_view view{cmd, size};
    auto iter = commands.find(view);

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

std::pair<DLLSafeCallback*, size_t> find_matching_command(std::wstring_view line) {
    auto iter = commands.find(NEXT_LINE);
    if (iter != commands.end()) {
        auto callback = iter->second;
        commands.erase(iter);
        return {callback, 0};
    }

    auto non_space = std::ranges::find_if_not(line, &std::iswspace);
    if (non_space == line.end()) {
        return {nullptr, 0};
    }

    auto cmd_end = std::find_if(non_space, line.end(), &std::iswspace);

    std::wstring cmd(cmd_end - non_space, '\0');
    std::transform(non_space, cmd_end, cmd.begin(), &std::towlower);

    return commands.contains(cmd) ? std::pair{commands[cmd], cmd_end - line.begin()}
                                  : std::pair{nullptr, 0};
}

#endif

}  // namespace impl

}  // namespace unrealsdk::commands
