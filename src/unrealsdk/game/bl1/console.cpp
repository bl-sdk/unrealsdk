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

// ############################################################################//
//  | CUSTOM COMMANDS |
// ############################################################################//

namespace bl1::cmd {

void sset_command_impl(const wchar_t* args, size_t, size_t);
void sgetnativefunc_command_impl(const wchar_t* args, size_t, size_t);

}  // namespace bl1::cmd

namespace {

// ############################################################################//
//  | DEFAULT CONSOLE HOOKS |
// ############################################################################//

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
    auto cls = hook.obj->Class;

    static const auto command_property =
        hook.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);

    static const auto history_prop = cls->find_prop_and_validate<UStrProperty>(L"History"_fn);
    static const auto history_top_prop =
        cls->find_prop_and_validate<UIntProperty>(L"HistoryTop"_fn);
    static const auto history_bot_prop =
        cls->find_prop_and_validate<UIntProperty>(L"HistoryBot"_fn);
    static const auto history_cur_prop =
        cls->find_prop_and_validate<UIntProperty>(L"HistoryCur"_fn);

    static const UFunction* purge_command_func =
        cls->find_func_and_validate(L"PurgeCommandFromHistory"_fn);
    static const UFunction* save_config_func = cls->find_func_and_validate(L"SaveConfig"_fn);

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

    LOG(INFO, L">>> {} <<<", line);

    try {
        callback->operator()(line.c_str(), line.size(), cmd_len);
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred while running a console command: {}", ex.what());
    }

    return true;
}

// Would prefer to call a native function where possible, however best I can tell, OutputText is
// actually implemented directly in unrealscript (along most of the console mechanics).
BoundFunction console_output_text{};

bool inject_console_hook(hook_manager::Details& hook) {
    // clang-format off
    hook_manager::remove_hook(
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

// ############################################################################//
//  | BL1HOOK METHODS |
// ############################################################################//

void BL1Hook::inject_console(void) {
    // clang-format off
    hook_manager::add_hook(
        L"Engine.Console:ShippingConsoleCommand",
        unrealsdk::hook_manager::Type::PRE,
        L"bl1_say_bypass_hook",
        &say_bypass_hook
    );
    hook_manager::add_hook(
        L"Engine.Console:ConsoleCommand",
        hook_manager::Type::PRE,
        L"bl1_console_command_hook",
        &console_command_hook
    );
    hook_manager::add_hook(
        L"WillowGame.WillowGameViewportClient:PostRender",
        hook_manager::Type::PRE,
        L"bl1_inject_console_hook",
        &inject_console_hook
    );
    // clang-format on

    commands::add_command(L"sset", &bl1::cmd::sset_command_impl);
    commands::add_command(L"sgetnativefunc", &bl1::cmd::sgetnativefunc_command_impl);
}

void BL1Hook::uconsole_output_text(const std::wstring& str) const {
    if (console_output_text.func == nullptr) {
        return;
    }

    if (bl1_cfg::is_locking_console_write()) {
        static std::recursive_mutex s_Mutex{};
        std::lock_guard<std::recursive_mutex> guard{s_Mutex};
        console_output_text.call<void, UStrProperty>(str);

    } else {
        console_output_text.call<void, UStrProperty>(str);
    }
}

bool BL1Hook::is_console_ready(void) const {
    return console_output_text.func != nullptr;
}

// ############################################################################//
//  | CUSTOM HOOKS |
// ############################################################################//

namespace {

// NOTE: src->get(prop) gives linker errors; we only need the value pointer here.
void* uproperty_get_value_ptr(UProperty* prop, UObject* src, int32_t index = 0) {
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(src) + prop->Offset_Internal
                                   + prop->ElementSize * index);
}

void uconsole_write(const std::wstring& text) {
    console_output_text.call<void, UStrProperty>(text);
}

void* uproperty_import_text(UProperty* prop,
                            const wchar_t* value,
                            void* write_to,
                            int32_t flags,
                            UObject* src,
                            void* err) {
    // These are mostly speculative; I would hope the bitflags are the same...
    constexpr auto INDEX_PRE_PROP_CHANGED = 0x11;
    constexpr auto INDEX_POST_PROP_CHANGED = 0x13;
    constexpr auto INDEX_IMPORT_TEXT = 0x54;
    constexpr auto RF_NeedInitialisation = 0x200;

    typedef void(__fastcall * uprop_changed)(UObject*, void*, UProperty*);
    typedef void*(__fastcall * uprop_import_text)(UProperty*, void*, const wchar_t*, void*, int32_t,
                                                  UObject*, void*);

    // Pre OnChange?
    if ((src->ObjectFlags & RF_NeedInitialisation) == 0) {
        reinterpret_cast<uprop_changed>(src->vftable[INDEX_PRE_PROP_CHANGED])(src, nullptr, prop);
    }

    // UProperty::ImportText(...)
    auto func = reinterpret_cast<uprop_import_text>(prop->vftable[INDEX_IMPORT_TEXT]);
    void* result = func(prop, nullptr, value, write_to, flags, src, err);

    // - NOTE -
    // ```C
    // if ((*(uint *)&piVar2->ObjectFlags & 0x4000) == 0) {
    // ...
    // if ((((*(uint *)&piVar2->ObjectFlags & lower_bits | this_upper_bits & upper_bits) == 0) &&
    //     ((lower_bits & upper_bits) != 0xffffffff)) || ((this_upper_bits & 0x20000000) != 0)) {
    // ```
    // This snippet would eventually cause a reload/reset on the object to defaults stored in the
    //  .upk we don't want that because we have just modified it and would like to keep our changes.
    //  So adding 0x4000 to the src->ObjectFlags stops 'src' from being reset. This probably
    //  introduces new issues or quirks though. Note that this snippet only flags the object for
    //  reloading.
    //
    // Ref: 0x005f988a,
    src->ObjectFlags |= 0x4000;  // RF_NewerVersionExists

    // Post OnChange?
    if ((src->ObjectFlags & RF_NeedInitialisation) == 0) {
        reinterpret_cast<uprop_changed>(src->vftable[INDEX_POST_PROP_CHANGED])(src, nullptr, prop);
    }

    return result;
}

}  // namespace

// ############################################################################//
//  | CUSTOM CONSOLE COMMANDS |
// ############################################################################//

void bl1::cmd::sset_command_impl(const wchar_t* line, size_t, size_t) {
    std::wstringstream ss{line};

    // sset CLASS NAME PROPERTY VALUE
    // sset WillowGame.PlayerClassDefinition gd_Brick.Character.CharacterClass_Brick FOV 90.0
    std::wstring ignore{};
    std::wstring cls{};
    std::wstring name{};
    std::wstring prop_name{};

    if (!(ss >> ignore >> cls >> name >> prop_name)) {
        uconsole_write(L"Invalid format for 'sset' command");
        return;
    }

    // From current position to the end of the string
    ss >> std::ws;  // Remove leading whitespace
    std::streampos pos = ss.tellg();
    std::wstring value{ss.str().substr(static_cast<std::wstring::size_type>(pos))};

    UObject* found = find_object(cls, name);

    if (found == nullptr) {
        uconsole_write(L"Failed to find object '" + name + L"'");
        return;
    }

    UProperty* prop = found->Class->find_prop(FName{prop_name});

    if (prop == nullptr) {
        uconsole_write(L"Failed to find property '" + prop_name + L"'");
        return;
    }

    void* value_ptr = uproperty_get_value_ptr(prop, found);
    uproperty_import_text(prop, value.c_str(), value_ptr, 1, found, nullptr);
}

void bl1::cmd::sgetnativefunc_command_impl(const wchar_t* line, size_t, size_t) {
    // - NOTE -
    // I don't expect this one to be used a lot its mostly for finding addresses of functions in
    // disassembly. The text output might not be useful since you can just add a breakpoint here
    // and copy the address directly.
    //

    std::wstringstream ss{line};
    std::wstring ignore;
    std::wstring cls_name;
    std::wstring func_name;

    if (!(ss >> ignore >> cls_name >> func_name)) {
        uconsole_write(L"Invalid format for sgetnativefunc");
        return;
    }

    UClass* found_cls = find_class(cls_name);

    // Couldn't find the class
    if (!found_cls) {
        uconsole_write(L"Could not find class '" + cls_name + L"'");
        return;
    }

    UFunction* found_func = found_cls->find_func_and_validate(FName{func_name});

    // Couldn't find the function
    if (!found_func) {
        uconsole_write(fmt::format(L"Failed to find function '{}' in '{}'", func_name,
                                   found_cls->get_path_name()));
        return;
    }

    std::wstring msg = fmt::format(L"Function address is '{:#010x}'",
                                   reinterpret_cast<uintptr_t>(found_func->Func));

    uconsole_write(msg);
    LOG(MISC, "[CONSOLE] ~ {}", msg);
}

}  // namespace unrealsdk::game

#endif
