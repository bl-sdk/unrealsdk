#include "pch.h"

#if defined(UE3) && defined(ARCH_X86)

#include "game/bl2.h"
#include "game/game_hook.h"
#include "hook_manager.h"
#include "memory.h"
#include "unreal/classes/properties/ustrproperty.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/bound_function.h"
#include "unreal/wrappers/gobjects.h"
#include "unreal/wrappers/wrapped_args.h"
#include "unrealsdk.h"

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

void BL2Hook::hook(void) {
    // Make sure to do antidebug asap
    hook_antidebug();

    GameHook::hook();

    hexedit_set_command();
    hexedit_array_limit();
    hexedit_array_limit_message();
    hook_say_bypass();
}

#pragma region AntiDebug

// NOLINTBEGIN(readability-identifier-naming)
// NOLINTNEXTLINE(modernize-use-using)  - need a typedef for calling conventions in msvc
typedef NTSTATUS(WINAPI* NtSetInformationThread_func)(
    HANDLE ThreadHandle,
    THREAD_INFORMATION_CLASS ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength);
// NOLINTNEXTLINE(modernize-use-using)
typedef NTSTATUS(WINAPI* NtQueryInformationProcess_func)(HANDLE ProcessHandle,
                                                         PROCESSINFOCLASS ProcessInformationClass,
                                                         PVOID ProcessInformation,
                                                         ULONG ProcessInformationLength,
                                                         PULONG ReturnLength);

static constexpr auto ThreadHideFromDebugger = static_cast<THREAD_INFORMATION_CLASS>(17);
static constexpr auto ProcessDebugObjectHandle = static_cast<PROCESSINFOCLASS>(30);
// NOLINTEND(readability-identifier-naming)

// NOLINTBEGIN(readability-identifier-naming)
static NtSetInformationThread_func NtSetInformationThread_ptr;
static NTSTATUS NTAPI NtSetInformationThread_hook(HANDLE ThreadHandle,
                                                  THREAD_INFORMATION_CLASS ThreadInformationClass,
                                                  PVOID ThreadInformation,
                                                  ULONG ThreadInformationLength) {
    // NOLINTEND(readability-identifier-naming)
    if (ThreadInformationClass == ThreadHideFromDebugger) {
        return STATUS_SUCCESS;
    }

    return NtSetInformationThread_ptr(ThreadHandle, ThreadInformationClass, ThreadInformation,
                                      ThreadInformationLength);
}
static_assert(std::is_same_v<decltype(&NtSetInformationThread_hook), NtSetInformationThread_func>,
              "NtSetInformationThread signature is incorrect");

// NOLINTBEGIN(readability-identifier-naming)
static NtQueryInformationProcess_func NtQueryInformationProcess_ptr;
static NTSTATUS WINAPI NtQueryInformationProcess_hook(HANDLE ProcessHandle,
                                                      PROCESSINFOCLASS ProcessInformationClass,
                                                      PVOID ProcessInformation,
                                                      ULONG ProcessInformationLength,
                                                      PULONG ReturnLength) {
    // NOLINTEND(readability-identifier-naming)
    if (ProcessInformationClass == ProcessDebugObjectHandle) {
        return STATUS_PORT_NOT_SET;
    }

    return NtQueryInformationProcess_ptr(ProcessHandle, ProcessInformationClass, ProcessInformation,
                                         ProcessInformationLength, ReturnLength);
}
static_assert(
    std::is_same_v<decltype(&NtQueryInformationProcess_hook), NtQueryInformationProcess_func>,
    "NtQueryInformationProcess signature is incorrect");

void BL2Hook::hook_antidebug(void) {
    MH_STATUS status = MH_OK;

    LPVOID target = nullptr;
    status = MH_CreateHookApiEx(L"ntdll", "NtSetInformationThread",
                                reinterpret_cast<LPVOID>(NtSetInformationThread_hook),
                                reinterpret_cast<LPVOID*>(&NtSetInformationThread_ptr), &target);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create NtSetInformationThread hook: %x", status);
    } else {
        status = MH_EnableHook(target);
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable NtSetInformationThread hook: %x", status);
        }
    }

    status = MH_CreateHookApiEx(L"ntdll", "NtQueryInformationProcess",
                                reinterpret_cast<LPVOID>(NtQueryInformationProcess_hook),
                                reinterpret_cast<LPVOID*>(&NtQueryInformationProcess_ptr), &target);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create NtQueryInformationProcess hook: %x", status);
    } else {
        status = MH_EnableHook(target);
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable NtQueryInformationProcess hook: %x", status);
        }
    }
}

#pragma endregion

#pragma region Hex Edits

void BL2Hook::hexedit_set_command(void) {
    static const Pattern SET_COMMAND_SIG{"\x83\xC4\x0C\x85\xC0\x75\x1A\x6A\x01\x8D",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto set_command = sigscan<uint8_t*>(SET_COMMAND_SIG);
    if (set_command == nullptr) {
        LOG(MISC, "Couldn't find set command signature, assuming already hex edited");
    } else {
        LOG(MISC, "Set Command: 0x%p", set_command);

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(set_command + 5, 2);
        set_command[5] = 0x90;
        set_command[6] = 0x90;
        // NOLINTEND(readability-magic-numbers)
    }
}

void BL2Hook::hexedit_array_limit(void) {
    static const Pattern ARRAY_LIMIT_SIG{"\x7E\x05\xB9\x64\x00\x00\x00\x3B\xF9\x0F\x8D",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto array_limit = sigscan<uint8_t*>(ARRAY_LIMIT_SIG);
    if (array_limit == nullptr) {
        LOG(MISC, "Couldn't find array limit signature, assuming already hex edited");
    } else {
        LOG(MISC, "Array Limit: 0x%p", array_limit);

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit, 1);
        array_limit[0] = 0xEB;
        // NOLINTEND(readability-magic-numbers)
    }
}

void BL2Hook::hexedit_array_limit_message(void) {
    static const Pattern ARRAY_LIMIT_MESSAGE{
        "\x0F\x8C\x7B\x00\x00\x00\x8B\x8D\x9C\xEE\xFF\xFF\x83\xC0\x9D\x50",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto array_limit_msg = sigscan<uint8_t*>(ARRAY_LIMIT_MESSAGE);
    if (array_limit_msg == nullptr) {
        LOG(MISC, "Couldn't find array limit message signature, assuming already hex edited");
    } else {
        LOG(MISC, "Array Limit Message: 0x%p", array_limit_msg);

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit_msg + 1, 1);
        array_limit_msg[1] = 0x85;
        // NOLINTEND(readability-magic-numbers)
    }
}

#pragma endregion

#pragma region Say Bypass

static bool shipping_console_command_hook(unreal::UFunction* /*func*/,
                                          unreal::UObject* obj,
                                          unreal::WrappedArgs& args) {
    static UFunction* console_command_func = nullptr;
    static UStrProperty* command_property = nullptr;

    // Optimize so we only call find once for each
    // TODO: can be removed once we have a general optimized `UStruct::find`
    if (console_command_func == nullptr) {
        console_command_func = obj->Class->find_and_validate<UFunction>(L"ConsoleCommand"_fn);
        command_property = args.type->find_and_validate<UStrProperty>(L"Command"_fn);
    }

    obj->get(console_command_func)
        .call<void, UStrProperty>(args.get<UStrProperty>(command_property));
    return true;
}

void BL2Hook::hook_say_bypass(void) {
    unrealsdk::hooks["Engine.Console:ShippingConsoleCommand"]["unrealsdk_bl2_say_bypass"] =
        &shipping_console_command_hook;
}

#pragma endregion

#pragma region ProcessEvent

// This function is actually thiscall, but MSVC won't let us declare static thiscall functions
// As a workaround, declare it fastcall, and add a dummy edx arg.
// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* process_event_func)(UObject* obj,
                                             void* /*edx*/,
                                             UFunction* func,
                                             void* params,
                                             void* result);

static process_event_func process_event_ptr;
static void __fastcall process_event_hook(UObject* obj,
                                          void* edx,
                                          UFunction* func,
                                          void* params,
                                          void* result) {
    try {
        WrappedArgs args{func, params};
        if (hook_manager::process_event(obj, func, args)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the ProcessEvent hook: %s", ex.what());
    }

    process_event_ptr(obj, edx, func, params, result);
}
static_assert(std::is_same_v<decltype(&process_event_hook), process_event_func>,
              "process_event signature is incorrect");

void BL2Hook::hook_process_event(void) {
    const Pattern PROCESS_EVENT_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x50\xA1"
        "\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00"
        "\x00\x8B\xF1\x89\x75\xEC",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF"
        "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"
        "\x00\xFF\xFF\xFF\xFF\xFF"};

    sigscan_and_detour(PROCESS_EVENT_SIG, process_event_hook, &process_event_ptr, "ProcessEvent");
}

void BL2Hook::process_event(unreal::UObject* object, unreal::UFunction* func, void* params) const {
    process_event_hook(object, nullptr, func, params, nullptr);
}

#pragma endregion

#pragma region CallFunction

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* call_function_func)(UObject* obj,
                                             void* /*edx*/,
                                             FFrame* stack,
                                             void* params,
                                             UFunction* func);

static call_function_func call_function_ptr;
static void __fastcall call_function_hook(UObject* obj,
                                          void* edx,
                                          FFrame* stack,
                                          void* result,
                                          UFunction* func) {
    try {
        if (hook_manager::call_function(obj, stack, result, func)) {
            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occured during the CallFunction hook: %s", ex.what());
    }

    call_function_ptr(obj, edx, stack, result, func);
}
static_assert(std::is_same_v<decltype(&call_function_hook), call_function_func>,
              "call_function signature is incorrect");

void BL2Hook::hook_call_function(void) {
    const Pattern CALL_FUNCTION_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00"
        "\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3"
        "\x00\x00\x00\x00\x8B\x7D\x10\x8B\x45\x0C",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"
        "\x00\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00"
        "\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    sigscan_and_detour(CALL_FUNCTION_SIG, call_function_hook, &call_function_ptr, "CallFunction");
}

#pragma endregion

#pragma region Globals

void BL2Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x8B\x40\x08",
                                      "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gobjects_instr = sigscan(GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);

    this->gobjects = GObjects(gobjects_ptr);
}

void BL2Hook::find_gnames(void) {
    static const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x83\x3C\x81\x00\x74\x5C",
                                    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gnames_instr = sigscan(GNAMES_SIG);
    auto gnames_ptr = read_offset<GNames::internal_type>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    this->gnames = GNames(gnames_ptr);
}

#pragma endregion

#pragma region FName::Init

void BL2Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x81\xEC\x9C\x0C",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    this->fname_init_ptr = sigscan<void*>(FNAME_INIT_SIG);
    LOG(MISC, "FName::Init: 0x%p", this->fname_init_ptr);
}

void BL2Hook::fname_init(FName* name, const std::wstring& str, int32_t number) const {
    this->fname_init(name, str.c_str(), number);
}
void BL2Hook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    // NOLINTNEXTLINE(modernize-use-using)  - need a typedef for the __thiscall
    typedef void*(__thiscall * fname_init_func)(FName * name, const wchar_t* str, int32_t number,
                                                int32_t find_type, int32_t split_name);

    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str, number, 1, 1);
}

#pragma endregion

#pragma region FFrame::Step

void BL2Hook::find_fframe_step(void) {
    static const Pattern FFRAME_STEP_SIG{"\x55\x8B\xEC\x8B\x41\x18\x0F\xB6\x10",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    this->fframe_step_ptr = sigscan<fframe_step_func>(FFRAME_STEP_SIG);
    LOG(MISC, "FFrame::Step: 0x%p", this->fframe_step_ptr);
}
void BL2Hook::fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const {
    this->fframe_step_ptr(frame, obj, param);
}

#pragma endregion

#pragma region FMemory

void BL2Hook::find_gmalloc(void) {
    static const Pattern GMALLOC_PATTERN{"\x00\x00\x00\x00\xFF\xD7\x83\xC4\x04\x89\x45\xE4",
                                         "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto sig_address = sigscan(GMALLOC_PATTERN);
    this->gmalloc = *read_offset<FMalloc**>(sig_address);
    LOG(MISC, "GMalloc: 0x%p", this->gmalloc);
}
void* BL2Hook::malloc(size_t len) const {
    auto ret = this->gmalloc->vftable->malloc(this->gmalloc, len, get_alignment(len));
    memset(ret, 0, len);
    return ret;
}
void* BL2Hook::realloc(void* original, size_t len) const {
    return this->gmalloc->vftable->realloc(this->gmalloc, original, len, get_alignment(len));
}
void BL2Hook::free(void* data) const {
    this->gmalloc->vftable->free(this->gmalloc, data);
}

#pragma endregion

#pragma region ConstructObject

void BL2Hook::find_construct_object(void) {
    static const Pattern CONSTRUCT_OBJECT_PATTERN{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x10\x53\x56"
        "\x57\xA1\x00\x00\x00\x00\x33\xC5\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\x7D\x08\x8A"
        "\x87",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF"
        "\xFF"};
    this->construct_obj_ptr = sigscan<construct_obj_func>(CONSTRUCT_OBJECT_PATTERN);
    LOG(MISC, "StaticConstructObject: 0x%p", this->construct_obj_ptr);
}

unreal::UObject* BL2Hook::construct_object(unreal::UClass* cls,
                                           unreal::UObject* outer,
                                           const unreal::FName& name,
                                           decltype(unreal::UObject::ObjectFlags) flags,
                                           unreal::UObject* template_obj) const {
    return this->construct_obj_ptr(cls, outer, name, flags, template_obj, nullptr, nullptr,
                                   0 /* false */);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
