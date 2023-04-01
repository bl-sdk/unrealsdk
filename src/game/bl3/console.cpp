#include "pch.h"

#include "env.h"
#include "game/bl3/bl3.h"
#include "hook_manager.h"
#include "unreal/classes/properties/copyable_property.h"
#include "unreal/classes/properties/uarrayproperty.h"
#include "unreal/classes/properties/uobjectproperty.h"
#include "unreal/classes/properties/ustructproperty.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/classes/uobject_funcs.h"
#include "unreal/structs/fname.h"
#include "unreal/structs/fstring.h"
#include "unreal/wrappers/gobjects.h"
#include "unreal/wrappers/wrapped_array.h"
#include "unreal/wrappers/wrapped_struct.h"
#include "unrealsdk.h"

#if defined(UE4) && defined(ARCH_X64)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

static const std::wstring INJECT_CONSOLE_FUNC = L"/Script/Engine.PlayerController:ClientSetHUD";
static const std::wstring INJECT_CONSOLE_ID = L"unrealsdk_bl3_inject_console";

static UObject* console = nullptr;

static bool inject_console_hook(hook_manager::HookDetails& hook) {
    hook_manager::hooks[INJECT_CONSOLE_FUNC].pre.erase(INJECT_CONSOLE_ID);

    auto local_player = hook.obj->get<UObjectProperty>(L"Player"_fn);
    auto viewport = local_player->get<UObjectProperty>(L"ViewportClient"_fn);
    auto console_property =
        viewport->Class->find_prop_and_validate<UObjectProperty>(L"ViewportConsole"_fn);
    console = viewport->get(console_property);

    if (console == nullptr) {
        auto default_console = console_property->get_property_class()->ClassDefaultObject;
        console = unrealsdk::construct_object(default_console->Class, default_console->Outer);
        viewport->set<UObjectProperty>(L"ViewportConsole"_fn, console);
    }

    console->set<UObjectProperty>(L"ConsoleTargetPlayer"_fn, local_player);

    LOG(MISC, "Injected console");

    // There isn't really a good path to the input settings class, which should be a singleton, so
    // just search through gobjects for the default object ¯\_(ツ)_/¯
    auto input_settings_fn = L"InputSettings"_fn;
    for (const auto& inner_obj : gobjects()) {
        if (inner_obj->Class->Name != input_settings_fn) {
            continue;
        }

        auto existing_console_key =
            inner_obj->get<UStructProperty>(L"ConsoleKey"_fn).get<UNameProperty>(L"KeyName"_fn);
        FName console_key{0, 0};

        if (existing_console_key != L"None"_fn || existing_console_key == L"Undefine"_fn) {
            LOG(MISC, "Console key is already set to {}", existing_console_key);

            console_key = existing_console_key;
        } else {
            auto wanted_console_key = env::get(env::CONSOLE_KEY, env::CONSOLE_KEY_DEFAULT);
            console_key = {wanted_console_key};

            inner_obj->get<UStructProperty>(L"ConsoleKey"_fn)
                .set<UNameProperty>(L"KeyName"_fn, console_key);

            LOG(MISC, "Set console key to '{}'", wanted_console_key);
        }

        // Make sure the array version is set to the same
        auto arr = inner_obj->get<UArrayProperty>(L"ConsoleKeys"_fn);
        arr.resize(1);
        arr.get_at<UStructProperty>(0).set<UNameProperty>(L"KeyName"_fn, console_key);
    }

    return false;
}

void BL3Hook::inject_console(void) {
    hook_manager::hooks[INJECT_CONSOLE_FUNC].pre[INJECT_CONSOLE_ID] = &inject_console_hook;
}

void BL3Hook::uconsole_output_text(const std::wstring& str) const {
    static constexpr auto DEFAULT_OUTPUT_TEXT_VF_INDEX = 83;

    static auto idx =
        env::get_numeric<size_t>(env::UCONSOLE_OUTPUT_TEXT_VF_INDEX, DEFAULT_OUTPUT_TEXT_VF_INDEX);

    if (console == nullptr) {
        return;
    }

    TemporaryFString fstr{str};
    console->call_virtual_function<void, TemporaryFString*>(idx, &fstr);
}

}  // namespace unrealsdk::game

#endif
