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
#include "unreal/wrappers/wrapped_args.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

static const std::wstring SAY_BYPASS_FUNC = L"Engine.Console:ShippingConsoleCommand";
static const std::wstring SAY_BYPASS_ID = L"unrealsdk_bl2_say_bypass";
static const std::wstring INJECT_CONSOLE_FUNC = L"WillowGame.WillowGameViewportClient:PostRender";
static const std::wstring INJECT_CONSOLE_ID = L"unrealsdk_bl2_inject_console";

static bool say_bypass_hook(UFunction* /*func*/, UObject* obj, WrappedArgs& args) {
    static UFunction* console_command_func = nullptr;
    static UStrProperty* command_property = nullptr;

    // Optimize so we only call find once for each
    if (console_command_func == nullptr) {
        console_command_func = obj->Class->find_and_validate<UFunction>(L"ConsoleCommand"_fn);
        command_property = args.type->find_and_validate<UStrProperty>(L"Command"_fn);
    }

    obj->get(console_command_func)
        .call<void, UStrProperty>(args.get<UStrProperty>(command_property));
    return true;
}

static BoundFunction console_output_text{};

static bool inject_console_hook(UFunction* /*func*/, UObject* obj, WrappedArgs& /*args*/) {
    hook_manager::hooks[INJECT_CONSOLE_FUNC].erase(INJECT_CONSOLE_ID);

    auto console = obj->get<UObjectProperty>(L"ViewportConsole"_fn);

    // Grab this reference ASAP
    console_output_text = console->get<UFunction>(L"OutputText"_fn);

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
    hook_manager::hooks[SAY_BYPASS_FUNC][SAY_BYPASS_ID] = &say_bypass_hook;
    hook_manager::hooks[INJECT_CONSOLE_FUNC][INJECT_CONSOLE_ID] = &inject_console_hook;
}

void BL2Hook::uconsole_output_text(const std::wstring& str) const {
    if (console_output_text.func == nullptr) {
        return;
    }

    console_output_text.call<void, UStrProperty>(str);
}

}  // namespace unrealsdk::game

#endif
