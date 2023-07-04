#ifndef UNREALSDK_SRC_UNREALSDK_COMMANDS_H
#define UNREALSDK_SRC_UNREALSDK_COMMANDS_H

#include "unrealsdk/pch.h"

namespace unrealsdk::commands {

/*
You can use this module to register custom console commands, in a single unified interface.

To register a command, you simply provide it's name. Commands are matched by comparing the first
block of non-whitespace characters in a line submitted to console against all registered names.

As a special case, if you register the special `NEXT_LINE` command, it will always match the very
next line, in place of anything else which might have been matched otherwise. It will then
immediately be removed (though before the callback is run, so you can re-register it if needed), to
allow normal command processing to continue afterwards.
*/

/**
 * @brief A special value used to register a command which will always match the very next line.
 * @note Only one matchall command can be registered at a time.
 * @note The matchall command is automatically removed after it gets matched.
 */
extern const std::wstring NEXT_LINE;

/**
 * @brief A callback for a custom console command.
 *
 * @param line The full line which triggered the callback - including any whitespace.
 * @param size The number of characters in the line, excluding the null terminator.
 * @param cmd_len The length of the matched command, including leading whitespace - `line[cmd_len]`
 *                points to the first whitespace char after the command (or off the end of the
 *                string if there was none). 0 in the case of a `NEXT_LINE` match.
 */
using Callback = void(const wchar_t* line, size_t size, size_t cmd_len) UNREALSDK_CAPI_SUFFIX;

/**
 * @brief Adds a custom console command.
 *
 * @param cmd The command to match.
 * @param callback The callback for when the command is run.
 * @return True if successfully added, false if an identical command already exists.
 */
bool add_command(const std::wstring& cmd, Callback* callback);

/**
 * @brief Check if a custom console command is registered.
 *
 * @param cmd The command to match.
 * @return True if the command is registered.
 */
bool has_command(const std::wstring& cmd);

/**
 * @brief Removes a custom console command.
 *
 * @param cmd The command to remove.
 * @return True if successfully removed, false if no such command exists.
 */
bool remove_command(const std::wstring& cmd);

namespace impl {  // These functions are only relevant when implementing a game hook
#ifndef UNREALSDK_IMPORTING

/**
 * @brief Finds the command which matches the given line.
 *
 * @param line The line which was submitted.
 * @return A pair of the callback to run and the offset to pass to it, or of nullptr and 0 if there
 *         was no match.
 */
std::pair<Callback*, size_t> find_matching_command(const std::wstring& line);

#endif
}  // namespace impl

}  // namespace unrealsdk::commands

#endif /* UNREALSDK_SRC_UNREALSDK_COMMANDS_H */
