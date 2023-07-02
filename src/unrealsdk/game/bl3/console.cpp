#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uobject_funcs.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

const std::wstring INJECT_CONSOLE_FUNC = L"/Script/Engine.PlayerController:ClientSetHUD";
const constexpr auto INJECT_CONSOLE_TYPE = hook_manager::Type::PRE;
const std::wstring INJECT_CONSOLE_ID = L"unrealsdk_bl3_inject_console";

UObject* console = nullptr;

bool inject_console_hook(hook_manager::Details& hook) {
    hook_manager::remove_hook(INJECT_CONSOLE_FUNC, INJECT_CONSOLE_TYPE, INJECT_CONSOLE_ID);

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
            auto wanted_console_key = env::get(env::CONSOLE_KEY, env::defaults::CONSOLE_KEY);
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

}  // namespace

void BL3Hook::inject_console(void) {
    hook_manager::add_hook(INJECT_CONSOLE_FUNC, INJECT_CONSOLE_TYPE, INJECT_CONSOLE_ID,
                           &inject_console_hook);
}

void BL3Hook::uconsole_output_text(const std::wstring& str) const {
    static auto idx = env::get_numeric<size_t>(env::UCONSOLE_OUTPUT_TEXT_VF_INDEX,
                                               env::defaults::UCONSOLE_OUTPUT_TEXT_VF_INDEX);

    if (console == nullptr) {
        return;
    }

    TemporaryFString fstr{str};
    console->call_virtual_function<void, TemporaryFString*>(idx, &fstr);
}

}  // namespace unrealsdk::game

#endif
