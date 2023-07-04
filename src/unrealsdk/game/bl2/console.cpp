#include "unrealsdk/pch.h"

#include "unrealsdk/commands.h"
#include "unrealsdk/env.h"
#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uobject_funcs.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

const std::wstring SAY_BYPASS_FUNC = L"Engine.Console:ShippingConsoleCommand";
const constexpr auto SAY_BYPASS_TYPE = hook_manager::Type::PRE;
const std::wstring SAY_BYPASS_ID = L"unrealsdk_bl2_say_bypass";

// We could combine this with the above, but by keeping them seperate it'll let users disable one if
// they really want to
const std::wstring CONSOLE_COMMAND_FUNC = L"Engine.Console:ConsoleCommand";
const constexpr auto CONSOLE_COMMAND_TYPE = hook_manager::Type::PRE;
const std::wstring CONSOLE_COMMAND_ID = L"unrealsdk_bl2_console_command";

const std::wstring INJECT_CONSOLE_FUNC = L"WillowGame.WillowGameViewportClient:PostRender";
const constexpr auto INJECT_CONSOLE_TYPE = hook_manager::Type::PRE;
const std::wstring INJECT_CONSOLE_ID = L"unrealsdk_bl2_inject_console";

bool say_bypass_hook(hook_manager::Details& hook) {
    static const auto console_command_func =
        hook.obj->Class->find_func_and_validate(L"ConsoleCommand"_fn);
    static const auto command_property =
        hook.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);

    hook.obj->get<UFunction, BoundFunction>(console_command_func)
        .call<void, UStrProperty>(hook.args->get<UStrProperty>(command_property));
    return true;
}

bool console_command_hook(hook_manager::Details& hook) {
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

    // Don't want to log this, just output to console by itself
    unrealsdk::uconsole_output_text(unrealsdk::fmt::format(L">>> {} <<<", line));

    callback(line.c_str(), line.size(), cmd_len);

    return true;
}

BoundFunction console_output_text{};

bool inject_console_hook(hook_manager::Details& hook) {
    hook_manager::remove_hook(INJECT_CONSOLE_FUNC, INJECT_CONSOLE_TYPE, INJECT_CONSOLE_ID);

    auto console = hook.obj->get<UObjectProperty>(L"ViewportConsole"_fn);

    // Grab this reference ASAP
    console_output_text = console->get<UFunction, BoundFunction>(L"OutputText"_fn);

    auto existing_console_key = console->get<UNameProperty>(L"ConsoleKey"_fn);
    if (existing_console_key != L"None"_fn || existing_console_key == L"Undefine"_fn) {
        LOG(MISC, "Console key is already set to '{}'", existing_console_key);
    } else {
        auto wanted_console_key = env::get(env::CONSOLE_KEY, env::defaults::CONSOLE_KEY);
        console->set<UNameProperty>(L"ConsoleKey"_fn, FName{wanted_console_key});

        LOG(MISC, "Set console key to '{}'", wanted_console_key);
    }

    return false;
}

}  // namespace

void BL2Hook::inject_console(void) {
    hook_manager::add_hook(SAY_BYPASS_FUNC, SAY_BYPASS_TYPE, SAY_BYPASS_ID, &say_bypass_hook);
    hook_manager::add_hook(CONSOLE_COMMAND_FUNC, CONSOLE_COMMAND_TYPE, CONSOLE_COMMAND_ID,
                           &console_command_hook);
    hook_manager::add_hook(INJECT_CONSOLE_FUNC, INJECT_CONSOLE_TYPE, INJECT_CONSOLE_ID,
                           &inject_console_hook);
}

void BL2Hook::uconsole_output_text(const std::wstring& str) const {
    if (console_output_text.func == nullptr) {
        return;
    }

    console_output_text.call<void, UStrProperty>(str);
}

}  // namespace unrealsdk::game

#endif
