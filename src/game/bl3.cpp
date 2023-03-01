#include "pch.h"

#if defined(UE4) && defined(ARCH_X64)

#include "env.h"
#include "game/bl3.h"
#include "game/game_hook.h"
#include "hook_manager.h"
#include "memory.h"
#include "unreal/classes/properties/copyable_property.h"
#include "unreal/classes/properties/uarrayproperty.h"
#include "unreal/classes/properties/uobjectproperty.h"
#include "unreal/classes/properties/ustructproperty.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/classes/uobject_funcs.h"
#include "unreal/structs/fframe.h"
#include "unreal/structs/fname.h"
#include "unreal/structs/tarray.h"
#include "unreal/wrappers/gobjects.h"
#include "unreal/wrappers/wrapped_args.h"
#include "unreal/wrappers/wrapped_struct.h"
#include "unrealsdk.h"

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

#pragma region ProcessEvent

using process_event_func = void(UObject* obj, UFunction* func, void* params);

static process_event_func* process_event_ptr;
void process_event_hook(UObject* obj, UFunction* func, void* params) {
    try {
        WrappedArgs args{func, params};
        if (hook_manager::process_event(obj, func, args)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the ProcessEvent hook: %s", ex.what());
    }

    process_event_ptr(obj, func, params);
}
static_assert(std::is_same_v<decltype(process_event_hook), process_event_func>,
              "process_event signature is incorrect");

void BL3Hook::hook_process_event(void) {
    const Pattern PROCESS_EVENT_SIG{
        "\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\xF0\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    sigscan_and_detour(PROCESS_EVENT_SIG, process_event_hook, &process_event_ptr, "ProcessEvent");
}

void BL3Hook::process_event(unreal::UObject* object, unreal::UFunction* func, void* params) const {
    process_event_hook(object, func, params);
}

#pragma endregion

#pragma region CallFunction

using call_function_func = void(UObject* obj, FFrame* stack, void* result, UFunction* func);

static call_function_func* call_function_ptr;
void call_function_hook(UObject* obj, FFrame* stack, void* result, UFunction* func) {
    try {
        if (hook_manager::call_function(obj, stack, result, func)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the CallFunction hook: %s", ex.what());
    }

    call_function_ptr(obj, stack, result, func);
}
static_assert(std::is_same_v<decltype(call_function_hook), call_function_func>,
              "call_function signature is incorrect");

void BL3Hook::hook_call_function(void) {
    const Pattern CALL_FUNCTION_SIG{
        "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xec\x28\x01\x00\x00\x48\x8d"
        "\x6c\x24\x30",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF"};

    sigscan_and_detour(CALL_FUNCTION_SIG, call_function_hook, &call_function_ptr, "CallFunction");
}

#pragma endregion

#pragma region Globals

void BL3Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{
        "\x48\x8D\x0D\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\xE8\x00\x00\x00\x00\xC6\x05",
        "\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF", 3};

    auto gobjects_instr = sigscan(GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);

    this->gobjects = GObjects(gobjects_ptr);
}

void BL3Hook::find_gnames(void) {
    static const Pattern GNAMES_SIG{
        "\xE8\x00\x00\x00\x00\x48\x00\x00\x48\x89\x1D\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83"
        "\xC4\x28\xC3\x00\xDB\x48\x89\x1D\x00\x00\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83\xC4"
        "\x00\xC3",
        "\xFF\x00\x00\x00\x00\xFF\x00\x00\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF"
        "\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF"
        "\x00\xFF",
        0xB};

    auto gnames_instr = sigscan(GNAMES_SIG);
    auto gnames_ptr = *read_offset<GNames::internal_type*>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    this->gnames = GNames(gnames_ptr);
}

#pragma endregion

#pragma region FName::Init

void BL3Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_PATTERN{
        "\x40\x53\x48\x83\xEC\x30\xC7\x44\x24\x00\x00\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00"};

    this->fname_init_ptr = sigscan<fname_init_func>(FNAME_INIT_PATTERN);
    LOG(MISC, "FName::Init: 0x%p", this->fname_init_ptr);
}

void BL3Hook::fname_init(FName* name, const std::wstring& str, int32_t number) const {
    this->fname_init(name, str.c_str(), number);
}
void BL3Hook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    *name = this->fname_init_ptr(str, number, 1);
}

#pragma endregion

#pragma region FFrame::Step

void BL3Hook::find_fframe_step(void) {
    static const Pattern FFRAME_STEP_SIG{"\x48\x8B\x41\x20\x4C\x8B\xD2\x48\x8B\xD1",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    this->fframe_step_ptr = sigscan<fframe_step_func>(FFRAME_STEP_SIG);
    LOG(MISC, "FFrame::Step: 0x%p", this->fframe_step_ptr);
}
void BL3Hook::fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const {
    this->fframe_step_ptr(frame, obj, param);
}

#pragma endregion

#pragma region FMemory

void BL3Hook::find_gmalloc(void) {
    static const Pattern MALLOC_PATTERN{
        "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xF9\x8B\xDA\x48\x8B\x0D\x00\x00\x00\x00"
        "\x48\x85\xC9",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
        "\xFF\xFF\xFF"};

    static const Pattern REALLOC_PATTERN{
        "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xF1\x41\x8B\xD8\x48"
        "\x8B\x0D\x00\x00\x00\x00\x48\x8B\xFA",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF"};

    static const Pattern FREE_PATTERN{
        "\x48\x85\xC9\x74\x00\x53\x48\x83\xEC\x20\x48\x8B\xD9\x48\x8B\x0D\x00\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00"};

    this->fmemory_malloc_ptr = sigscan<fmemory_malloc_func>(MALLOC_PATTERN);
    this->fmemory_realloc_ptr = sigscan<fmemory_realloc_func>(REALLOC_PATTERN);
    this->fmemory_free_ptr = sigscan<fmemory_free_func>(FREE_PATTERN);

    LOG(MISC, "FMemory::Malloc: 0x%p", this->fmemory_malloc_ptr);
    LOG(MISC, "FMemory::Realloc: 0x%p", this->fmemory_realloc_ptr);
    LOG(MISC, "FMemory::Free: 0x%p", this->fmemory_free_ptr);
}
void* BL3Hook::malloc(size_t len) const {
    auto ret = this->fmemory_malloc_ptr(len, get_alignment(len));
    memset(ret, 0, len);
    return ret;
}
void* BL3Hook::realloc(void* original, size_t len) const {
    return this->fmemory_realloc_ptr(original, len, get_alignment(len));
}
void BL3Hook::free(void* data) const {
    this->fmemory_free_ptr(data);
}

#pragma endregion

#pragma region ConstructObject

void BL3Hook::find_construct_object(void) {
    static const Pattern CONSTRUCT_OBJECT_PATTERN{
        "\xE8\x00\x00\x00\x00\x41\x89\x3E\x4D\x8D\x46\x04",
        "\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 1};

    auto construct_obj_instr = sigscan(CONSTRUCT_OBJECT_PATTERN);
    this->construct_obj_ptr = read_offset<construct_obj_func>(construct_obj_instr);
    LOG(MISC, "StaticConstructObject: 0x%p", this->construct_obj_ptr);
}

unreal::UObject* BL3Hook::construct_object(unreal::UClass* cls,
                                           unreal::UObject* outer,
                                           const unreal::FName& name,
                                           decltype(unreal::UObject::ObjectFlags) flags,
                                           unreal::UObject* template_obj) const {
    return this->construct_obj_ptr(cls, outer, name, flags, 0, template_obj, 0 /* false */, nullptr,
                                   0 /* false */);
}

#pragma endregion

#pragma region Inject Console

static const std::string INJECT_CONSOLE_FUNC = "/Script/Engine.PlayerController:ClientSetHUD";
static const std::string INJECT_CONSOLE_ID = "unrealsdk_bl3_inject_console";

static UObject* console = nullptr;

static bool inject_console_hook(unreal::UFunction* /*func*/,
                                unreal::UObject* obj,
                                unreal::WrappedArgs& /*args*/) {
    unrealsdk::hooks[INJECT_CONSOLE_FUNC].erase(INJECT_CONSOLE_ID);

    auto local_player = obj->get<UObjectProperty>(L"Player"_fn);
    auto viewport = local_player->get<UObjectProperty>(L"ViewportClient"_fn);
    auto console_property =
        viewport->Class->find_and_validate<UObjectProperty>(L"ViewportConsole"_fn);
    console = viewport->get(console_property);

    if (console == nullptr) {
        auto default_console = console_property->get_property_class()->ClassDefaultObject;
        console = game::construct_object(default_console->Class, default_console->Outer);
        viewport->set<UObjectProperty>(L"ViewportConsole"_fn, console);
    }

    console->set<UObjectProperty>(L"ConsoleTargetPlayer"_fn, local_player);

    LOG(MISC, "Injected console");

    // There isn't really a good path to the input settings class, which should be a singleton, so
    // just search through gobjects for the default object ¯\_(ツ)_/¯
    auto input_settings_fn = L"InputSettings"_fn;
    for (const auto& inner_obj : game::gobjects()) {
        if (inner_obj->Class->Name != input_settings_fn) {
            continue;
        }

        auto existing_console_key =
            inner_obj->get<UStructProperty>(L"ConsoleKey"_fn).get<UNameProperty>(L"KeyName"_fn);
        unreal::FName console_key{0, 0};

        if (existing_console_key != L"None"_fn || existing_console_key == L"Undefine"_fn) {
            LOG(MISC, "Console key is already set to '%s'",
                std::string{existing_console_key}.c_str());

            console_key = existing_console_key;
        } else {
            auto wanted_console_key = env::get(env::CONSOLE_KEY, env::CONSOLE_KEY_DEFAULT);
            console_key = {wanted_console_key};

            inner_obj->get<UStructProperty>(L"ConsoleKey"_fn)
                .set<UNameProperty>(L"KeyName"_fn, console_key);

            LOG(MISC, "Set console key to '%s'", wanted_console_key.c_str());
        }

        // Make sure the array version is set to the same
        auto arr = inner_obj->get<UArrayProperty>(L"ConsoleKeys"_fn);
        arr.resize(1);
        arr.get_at<UStructProperty>(0).set<UNameProperty>(L"KeyName"_fn, console_key);
    }

    return false;
}

void BL3Hook::inject_console(void) const {
    unrealsdk::hooks[INJECT_CONSOLE_FUNC][INJECT_CONSOLE_ID] = &inject_console_hook;
}

void BL3Hook::uconsole_output_text(const std::wstring& str) const {
    static constexpr auto DEFAULT_OUTPUT_TEXT_VF_INDEX = 83;

    if (console == nullptr) {
        return;
    }

    auto size = str.size();
    if (size > TArray<void>::MAX_CAPACITY) {
        throw std::length_error("Tried to log a string longer than TArray max capacity!");
    }
    auto narrowed_size = static_cast<decltype(TArray<void>::count)>(size);

    // We know the input string exists for the lifetime of this function, and we know the string we
    // send to unreal will have a smaller lifetime within it.
    // Rather than use a more well defined FString type, this means we can just reference the stl
    // string's data directly, avoiding an extra copy.
    TArray<const wchar_t> fstr{str.data(), narrowed_size, narrowed_size};

    auto idx =
        env::get_numeric<size_t>(env::UCONSOLE_OUTPUT_TEXT_VF_INDEX, DEFAULT_OUTPUT_TEXT_VF_INDEX);
    console->call_virtual_function<void>(idx, &fstr);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
