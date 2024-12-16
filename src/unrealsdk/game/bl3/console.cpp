#include "unrealsdk/pch.h"

#include "unrealsdk/commands.h"
#include "unrealsdk/config.h"
#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uobject_funcs.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
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

const constexpr auto MAX_HISTORY_ENTRIES = 50;

UObject* console = nullptr;

void static_uconsole_output_text(const std::wstring& str) {
    static auto idx = config::get_int("unrealsdk.uconsole_output_text_vf_index")
                          .value_or(81);  // NOLINT(readability-magic-numbers)

    if (console == nullptr) {
        return;
    }

    // UConsole::OutputText does not print anything on a completely empty line - we would prefer to
    // call UConsole::OutputTextLine, but that got completely inlined
    // If we have an empty string, replace with with a single newline.
    static const std::wstring newline = L"\n";
    TemporaryFString fstr{str.empty() ? newline : str};
    console->call_virtual_function<void, TemporaryFString*>(idx, &fstr);
}

using console_command_func = void(UObject* console_obj, UnmanagedFString* raw_line);
console_command_func* console_command_ptr;

void console_command_hook(UObject* console_obj, UnmanagedFString* raw_line) {
    try {
        std::wstring line = *raw_line;

        auto [callback, cmd_len] = commands::impl::find_matching_command(line);
        if (callback != nullptr) {
            // Update the history buffer
            {
                // History buffer is oldest at index 0, newest at count
                // UE behavior is to remove the current line if it exists, then add it at the
                // bottom, and finally trim any overflowing entries from the top

                // Since we know we can never overflow if the current line already exists in the
                // buffer, we can combine the two memmoves

                auto history_buffer = console_obj->get<UArrayProperty>(L"HistoryBuffer"_fn);
                auto history_size = history_buffer.size();

                size_t matching_idx = history_size;
                for (size_t i = 0; i < history_size; i++) {
                    if (history_buffer.get_at<UStrProperty>(i) == line) {
                        matching_idx = i;
                    }
                }

                bool needs_move{};
                size_t dropped_idx{};

                // If no match
                if (matching_idx == history_size) {
                    // If we're exactly at max size, we'll need to move too, since we're about to
                    // add one
                    needs_move = history_size >= MAX_HISTORY_ENTRIES;
                    dropped_idx = 0;
                } else {
                    needs_move = true;
                    dropped_idx = matching_idx;
                }

                if (needs_move) {
                    history_buffer.destroy_at<UStrProperty>(dropped_idx);

                    // If the entry to drop is right at the end of the array, don't bother moving
                    // anything, just lower the count
                    if (dropped_idx != (history_size - 1)) {
                        auto data = reinterpret_cast<uintptr_t>(history_buffer.base->data);
                        auto element_size = history_buffer.type->ElementSize;

                        auto dest = data + (dropped_idx * element_size);
                        auto remaining_size = (history_size - dropped_idx) * element_size;
                        memmove(reinterpret_cast<void*>(dest),
                                reinterpret_cast<void*>(dest + element_size), remaining_size);
                    }

                    history_size--;
                }

                history_buffer.resize(history_size + 1);
                history_buffer.set_at<UStrProperty>(history_size, line);

                // UE would normally call UObject::SaveConfig here, but it's a pain to get to for
                // just this, and we'll see if people actually complain
            }

            /*
            This is a little awkward.
            Since we can't let execution though to the unreal function, we're responsible for
            printing the executed command line.

            We do this via output text directly, rather than the LOG macro, so that it's not
            affected by the console log level, and so that it happens immediately (the LOG macro is
            queued, and can get out of order with respect to native engine messages).

            However, for custom console commands it's also nice to see what the command was in the
            log file, since you'll see all their output too.

            We don't really expose a "write to log file only", since it's not usually something
            useful, so as a compromise just use the LOG macro on the lowest possible log level, and
            assume the lowest people practically set their console log level to is dev warning.
            */
            auto msg = unrealsdk::fmt::format(L">>> {} <<<", line);
            static_uconsole_output_text(msg);
            LOG(MIN, L"{}", msg);

            try {
                callback->operator()(line.c_str(), line.size(), cmd_len);
            } catch (const std::exception& ex) {
                LOG(ERROR, "An exception occurred while running a console command: {}", ex.what());
            }

            return;
        }

    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the ConsoleCommand hook: {}", ex.what());
    }

    console_command_ptr(console_obj, raw_line);
}

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

    static auto console_command_vf_idx =
        config::get_int("unrealsdk.uconsole_console_command_vf_index")
            .value_or(81);  // NOLINT(readability-magic-numbers)

    memory::detour(console->vftable[console_command_vf_idx], console_command_hook,
                   &console_command_ptr, "ConsoleCommand");

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
            std::string wanted_console_key{
                config::get_str("unrealsdk.console_key").value_or("Tilde")};
            console_key = FName{wanted_console_key};

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
    static_uconsole_output_text(str);
}

bool BL3Hook::is_console_ready(void) const {
    return console != nullptr;
}

}  // namespace unrealsdk::game

#endif
