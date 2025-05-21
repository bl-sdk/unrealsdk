#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"

#include "unrealsdk/commands.h"
#include "unrealsdk/config.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uobject_funcs.h"
#include "unrealsdk/unreal/find_class.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

// This is an extra hook, which we don't strictly need for the interface, but is really handy. By
// default the game prepends 'say ' to every command as a primitive way to disable console. Bypass
// it so you can actually use it.

const std::wstring SAY_BYPASS_FUNC = L"Engine.Console:ShippingConsoleCommand";
const constexpr auto SAY_BYPASS_TYPE = hook_manager::Type::PRE;
const std::wstring SAY_BYPASS_ID = L"unrealsdk_bl1_say_bypass";

// We could combine this with the say bypass, but by keeping them separate it'll let users disable
// one if they really want to
const std::wstring CONSOLE_COMMAND_FUNC = L"Engine.Console:ConsoleCommand";
// This is the actual end point of all console commands, the above function normally calls through
// into this one - but we needed to hook it to be able to manage the console history. If something
/// directly calls `PC.ConsoleCommand("my_cmd")`, we need this hook to be able to catch it.
const std::wstring PC_CONSOLE_COMMAND_FUNC = L"Engine.PlayerController:ConsoleCommand";

const constexpr auto CONSOLE_COMMAND_TYPE = hook_manager::Type::PRE;
const std::wstring CONSOLE_COMMAND_ID = L"unrealsdk_bl1_console_command";

const std::wstring INJECT_CONSOLE_FUNC = L"WillowGame.WillowGameViewportClient:PostRender";
const constexpr auto INJECT_CONSOLE_TYPE = hook_manager::Type::PRE;
const std::wstring INJECT_CONSOLE_ID = L"unrealsdk_bl1_inject_console";

BoundFunction console_output_text{};

bool say_bypass_hook(const hook_manager::Details& hook) {
    static const auto console_command_func =
        hook.obj->Class()->find_func_and_validate(L"ConsoleCommand"_fn);
    static const auto command_property =
        hook.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);

    hook.obj->get<UFunction, BoundFunction>(console_command_func)
        .call<void, UStrProperty>(hook.args->get<UStrProperty>(command_property));
    return true;
}

bool console_command_hook(const hook_manager::Details& hook) {
    static const auto command_property =
        hook.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);

    static const auto history_prop =
        hook.obj->Class()->find_prop_and_validate<UStrProperty>(L"History"_fn);
    static const auto history_top_prop =
        hook.obj->Class()->find_prop_and_validate<UIntProperty>(L"HistoryTop"_fn);
    static const auto history_bot_prop =
        hook.obj->Class()->find_prop_and_validate<UIntProperty>(L"HistoryBot"_fn);
    static const auto history_cur_prop =
        hook.obj->Class()->find_prop_and_validate<UIntProperty>(L"HistoryCur"_fn);

    static const UFunction* purge_command_func =
        hook.obj->Class()->find_func_and_validate(L"PurgeCommandFromHistory"_fn);
    static const UFunction* save_config_func =
        hook.obj->Class()->find_func_and_validate(L"SaveConfig"_fn);

    auto line = hook.args->get<UStrProperty>(command_property);

    auto [callback, cmd_len] = commands::impl::find_matching_command(line);
    if (callback == nullptr) {
        return false;
    }

    // Add to the history buffer
    {
        // History is a ring buffer of recent commands
        // HistoryBot points to the oldest entry, or -1 if history is empty.
        // HistoryTop points to the next entry to fill. If history is empty it's 0. The value it
        //  points at is *not* shown in the history if we've wrapped.
        // HistoryCur is always set to the same as top after running a command - presumably it
        //  changes while scrolling, but we can't really check that

        // First remove it from history
        hook.obj->get<UFunction, BoundFunction>(purge_command_func).call<void, UStrProperty>(line);

        // Insert this line at top
        auto history_top = hook.obj->get<UIntProperty>(history_top_prop);
        hook.obj->set<UStrProperty>(history_prop, history_top, line);

        // Increment top
        history_top = (history_top + 1) % history_prop->ArrayDim();
        hook.obj->set<UIntProperty>(history_top_prop, history_top);
        // And set current
        hook.obj->set<UIntProperty>(history_cur_prop, history_top);

        // Increment bottom if needed
        auto history_bot = hook.obj->get<UIntProperty>(history_bot_prop);
        if ((history_bot == -1) || history_bot == history_top) {
            hook.obj->set<UIntProperty>(history_bot_prop,
                                        (history_bot + 1) % history_prop->ArrayDim());
        }

        hook.obj->get<UFunction, BoundFunction>(save_config_func).call<void>();
    }

    /*
     * This is a little awkward.
     * Since we can't let execution though to the unreal function, we're responsible for printing
     * the executed command line.
     *
     * We do this via output text directly, rather than the LOG macro, so that it's not affected by
     * the console log level, and so that it happens immediately (the LOG macro is queued, and can
     * get out of order with respect to native engine messages).
     *
     * However, for custom console commands it's also nice to see what the command was in the log
     * file, since you'll see all their output too.
     *
     * We don't really expose a "write to log file only", since it's not usually something useful,
     * so as a compromise just use the LOG macro on the lowest possible log level, and assume the
     * lowest people practically set their console log level to is dev warning.
     */
    auto msg = format(L">>> {} <<<", line);
    console_output_text.call<void, UStrProperty>(msg);
    LOG(MIN, L"{}", msg);

    try {
        callback->operator()(line.c_str(), line.size(), cmd_len);
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred while running a console command: {}", ex.what());
    }

    return true;
}

bool pc_console_command_hook(const hook_manager::Details& hook) {
    static const auto command_property =
        hook.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);

    auto line = hook.args->get<UStrProperty>(command_property);

    auto [callback, cmd_len] = commands::impl::find_matching_command(line);
    if (callback == nullptr) {
        return false;
    }

    // This hook does not go to console, so there's no extra processing to be done, we can just run
    // the callback immediately
    try {
        callback->operator()(line.c_str(), line.size(), cmd_len);
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred while running a console command: {}", ex.what());
    }
    return true;
}

bool inject_console_hook(const hook_manager::Details& hook) {
    remove_hook(INJECT_CONSOLE_FUNC, INJECT_CONSOLE_TYPE, INJECT_CONSOLE_ID);

    auto console = hook.obj->get<UObjectProperty>(L"ViewportConsole"_fn);

    // Grab this reference ASAP
    // Actually using OutputTextLine because it handles an empty string - OutputText does nothing
    console_output_text = console->get<UFunction, BoundFunction>(L"OutputTextLine"_fn);

    auto existing_console_key = console->get<UNameProperty>(L"ConsoleKey"_fn);
    if (existing_console_key != L"None"_fn || existing_console_key == L"Undefine"_fn) {
        LOG(MISC, "Console key is already set to '{}'", existing_console_key);
    } else {
        std::string wanted_console_key{config::get_str("unrealsdk.console_key").value_or("Tilde")};
        console->set<UNameProperty>(L"ConsoleKey"_fn, FName{wanted_console_key});

        LOG(MISC, "Set console key to '{}'", wanted_console_key);
    }

    return false;
}

}  // namespace

void BL1Hook::inject_console(void) {
    add_hook(SAY_BYPASS_FUNC, SAY_BYPASS_TYPE, SAY_BYPASS_ID, &say_bypass_hook);

    add_hook(CONSOLE_COMMAND_FUNC, CONSOLE_COMMAND_TYPE, CONSOLE_COMMAND_ID, &console_command_hook);
    add_hook(PC_CONSOLE_COMMAND_FUNC, CONSOLE_COMMAND_TYPE, CONSOLE_COMMAND_ID,
             &pc_console_command_hook);

    add_hook(INJECT_CONSOLE_FUNC, INJECT_CONSOLE_TYPE, INJECT_CONSOLE_ID, &inject_console_hook);
}

void BL1Hook::uconsole_output_text(const std::wstring& str) const {
    if (console_output_text.func == nullptr) {
        return;
    }
    console_output_text.call<void, UStrProperty>(str);
}

bool BL1Hook::is_console_ready(void) const {
    return console_output_text.func != nullptr;
}

}  // namespace unrealsdk::game

#endif
