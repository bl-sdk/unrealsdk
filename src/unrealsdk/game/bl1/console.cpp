#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"

#include "unrealsdk/commands.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/find_class.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING) \
    && defined(UNREALSDK_GAME_BL1)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

BoundFunction console_output_text{};

////////////////////////////////////////////////////////////////////////////////
// | CONSOLE HOOKS |
////////////////////////////////////////////////////////////////////////////////

// - NOTE -
// I don't know if the say_crash_fix_hook is needed; don't know if the implementation/fix
// needs to change either. Just going assume it is not needed until someone complains about crashing
// in multiplayer lol.

bool say_bypass_hook(const hook_manager::Details& hook) {
    static const auto console_command_func =
        hook.obj->Class->find_func_and_validate(L"ConsoleCommand"_fn);
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
        hook.obj->Class->find_prop_and_validate<UStrProperty>(L"History"_fn);
    static const auto history_top_prop =
        hook.obj->Class->find_prop_and_validate<UIntProperty>(L"HistoryTop"_fn);
    static const auto history_bot_prop =
        hook.obj->Class->find_prop_and_validate<UIntProperty>(L"HistoryBot"_fn);
    static const auto history_cur_prop =
        hook.obj->Class->find_prop_and_validate<UIntProperty>(L"HistoryCur"_fn);

    static const UFunction* purge_command_func =
        hook.obj->Class->find_func_and_validate(L"PurgeCommandFromHistory"_fn);
    static const UFunction* save_config_func =
        hook.obj->Class->find_func_and_validate(L"SaveConfig"_fn);

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
        history_top = (history_top + 1) % history_prop->ArrayDim;
        hook.obj->set<UIntProperty>(history_top_prop, history_top);
        // And set current
        hook.obj->set<UIntProperty>(history_cur_prop, history_top);

        // Increment bottom if needed
        auto history_bot = hook.obj->get<UIntProperty>(history_bot_prop);
        if ((history_bot == -1) || history_bot == history_top) {
            hook.obj->set<UIntProperty>(history_bot_prop,
                                        (history_bot + 1) % history_prop->ArrayDim);
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
    // clang-format off
    remove_hook(
        L"WillowGame.WillowGameViewportClient:PostRender",
        hook_manager::Type::PRE,
        L"bl1_inject_console_hook"
    );
    // clang-format on

    auto console = hook.obj->get<UObjectProperty>(L"ViewportConsole"_fn);

    // Grab this reference ASAP
    // Actually using OutputTextLine because it handles an empty string - OutputText does nothing
    console_output_text = console->get<UFunction, BoundFunction>(L"OutputTextLine"_fn);

    auto existing_console_key = console->get<UNameProperty>(L"ConsoleKey"_fn);
    if (existing_console_key != L"None"_fn || existing_console_key == L"Undefine"_fn) {
        LOG(MISC, "Console key is already set to '{}'", existing_console_key);
    } else {
        auto wanted_console_key = bl1_cfg::console_key();
        console->set<UNameProperty>(L"ConsoleKey"_fn, FName{wanted_console_key});

        LOG(MISC, "Set console key to '{}'", wanted_console_key);
    }

    return false;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// | BL1HOOK ENTRIES |
////////////////////////////////////////////////////////////////////////////////

void BL1Hook::inject_console(void) {
    // clang-format off
    add_hook(
        L"Engine.Console:ShippingConsoleCommand",
        hook_manager::Type::PRE,
        L"bl1_say_bypass_hook",
        &say_bypass_hook
    );
    add_hook(
        L"Engine.Console:ConsoleCommand",
        hook_manager::Type::PRE,
        L"bl1_console_command_hook",
        &console_command_hook
    );
    add_hook(
        L"Engine.PlayerController:ConsoleCommand",
        hook_manager::Type::PRE,
        L"bl1_pc_console_command_hook",
        &pc_console_command_hook
    );
    add_hook(
        L"WillowGame.WillowGameViewportClient:PostRender",
        hook_manager::Type::PRE,
        L"bl1_inject_console_hook",
        &inject_console_hook
    );
    // clang-format on
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
