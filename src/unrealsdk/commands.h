#ifndef UNREALSDK_COMMANDS_H
#define UNREALSDK_COMMANDS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::commands {

/*
You can use this module to register custom console commands, in a single unified interface.

To register a command, you simply provide it's name. Commands are matched by comparing the first
block of non-whitespace characters in a line submitted to console against all registered names.

To create interactive menus, rather than registering separate commands for every input, use the
special `NEXT_LINE` command, which always matches the very next line, with higher priority than
anything else. These commands are only matched once, and are automatically removed before running
the callback. They also try to only match direct user inputs in console, and try ignore commands
sent via automated means - e.g. if the game actually sends an `open` command to load a map.
*/

/**
 * @brief A special value used to register a command which will always match the very next line.
 * @note Only one next line command can be registered at a time.
 * @note The next line command is automatically removed after it gets matched.
 * @note The next line command *should not* be matched by automated commands.
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
using DLLSafeCallback = utils::DLLSafeCallback<void, const wchar_t*, size_t, size_t>;
using Callback = DLLSafeCallback::InnerFunc;

/**
 * @brief Adds a custom console command.
 *
 * @param cmd The command to match.
 * @param callback The callback for when the command is run.
 * @return True if successfully added, false if an identical command already exists.
 */
bool add_command(std::wstring_view cmd, const Callback& callback);

/**
 * @brief Check if a custom console command is registered.
 *
 * @param cmd The command to match.
 * @return True if the command is registered.
 */
bool has_command(std::wstring_view cmd);

/**
 * @brief Removes a custom console command.
 *
 * @param cmd The command to remove.
 * @return True if successfully removed, false if no such command exists.
 */
bool remove_command(std::wstring_view cmd);

namespace impl {  // These functions are only relevant when implementing a game hook

#ifndef UNREALSDK_IMPORTING

/**
 * @brief Checks if a command line contains a valid command to be run.
 *
 * @param line The line to check.
 * @param direct_user_input True if this command was directly input to console by a user.
 * @return True if this contains a command we should run.
 */
bool is_command_valid(std::wstring_view line, bool direct_user_input);

/**
 * @brief Runs the command associated with the given line.
 * @note Assumes is_command_valid previously returned true on the same line.
 *
 * @param line The line holding the command to be run.
 */
void run_command(std::wstring_view line);

#endif
}  // namespace impl

}  // namespace unrealsdk::commands

#endif /* UNREALSDK_COMMANDS_H */
