#include "unrealsdk/pch.h"

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
const std::wstring INJECT_CONSOLE_FUNC = L"WillowGame.WillowGameViewportClient:PostRender";
const constexpr auto INJECT_CONSOLE_TYPE = hook_manager::Type::PRE;
const std::wstring INJECT_CONSOLE_ID = L"unrealsdk_bl2_inject_console";

bool say_bypass_hook(hook_manager::Details& hook) {
    static UFunction* console_command_func = nullptr;
    static UStrProperty* command_property = nullptr;

    // Optimize so we only call find once for each
    if (console_command_func == nullptr) {
        console_command_func = hook.obj->Class->find_func_and_validate(L"ConsoleCommand"_fn);
        command_property = hook.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);
    }

    hook.obj->get<UFunction, BoundFunction>(console_command_func)
        .call<void, UStrProperty>(hook.args->get<UStrProperty>(command_property));
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
