#include "pch.h"

#include "env.h"
#include "game/bl2/bl2.h"
#include "hook_manager.h"
#include "unreal/classes/properties/copyable_property.h"
#include "unreal/classes/properties/uobjectproperty.h"
#include "unreal/classes/properties/ustrproperty.h"
#include "unreal/classes/uclass.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/classes/uobject_funcs.h"
#include "unreal/structs/fname.h"
#include "unreal/wrappers/bound_function.h"
#include "unreal/wrappers/wrapped_struct.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

static const std::wstring SAY_BYPASS_FUNC = L"Engine.Console:ShippingConsoleCommand";
static const std::wstring SAY_BYPASS_ID = L"unrealsdk_bl2_say_bypass";
static const std::wstring INJECT_CONSOLE_FUNC = L"WillowGame.WillowGameViewportClient:PostRender";
static const std::wstring INJECT_CONSOLE_ID = L"unrealsdk_bl2_inject_console";

static bool say_bypass_hook(hook_manager::HookDetails& hook) {
    static UFunction* console_command_func = nullptr;
    static UStrProperty* command_property = nullptr;

    // Optimize so we only call find once for each
    if (console_command_func == nullptr) {
        console_command_func = hook.obj->Class->find_func_and_validate(L"ConsoleCommand"_fn);
        command_property = hook.args.type->find_prop_and_validate<UStrProperty>(L"Command"_fn);
    }

    hook.obj->get<UFunction, BoundFunction>(console_command_func)
        .call<void, UStrProperty>(hook.args.get<UStrProperty>(command_property));
    return true;
}

static BoundFunction console_output_text{};

static bool inject_console_hook(hook_manager::HookDetails& hook) {
    hook_manager::hooks[INJECT_CONSOLE_FUNC].pre.erase(INJECT_CONSOLE_ID);

    auto console = hook.obj->get<UObjectProperty>(L"ViewportConsole"_fn);

    // Grab this reference ASAP
    console_output_text = console->get<UFunction, BoundFunction>(L"OutputText"_fn);

    auto existing_console_key = console->get<UNameProperty>(L"ConsoleKey"_fn);
    if (existing_console_key != L"None"_fn || existing_console_key == L"Undefine"_fn) {
        LOG(MISC, "Console key is already set to '{}'", existing_console_key);
    } else {
        auto wanted_console_key = env::get(env::CONSOLE_KEY, env::CONSOLE_KEY_DEFAULT);
        console->set<UNameProperty>(L"ConsoleKey"_fn, FName{wanted_console_key});

        LOG(MISC, "Set console key to '{}'", wanted_console_key);
    }

    return false;
}

void BL2Hook::inject_console(void) {
    hook_manager::hooks[SAY_BYPASS_FUNC].pre[SAY_BYPASS_ID] = &say_bypass_hook;
    hook_manager::hooks[INJECT_CONSOLE_FUNC].pre[INJECT_CONSOLE_ID] = &inject_console_hook;
}

void BL2Hook::uconsole_output_text(const std::wstring& str) const {
    if (console_output_text.func == nullptr) {
        return;
    }

    console_output_text.call<void, UStrProperty>(str);
}

}  // namespace unrealsdk::game

#endif
