
#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"

#include <unrealsdk/commands.h>
#include <unrealsdk/hook_manager.h>
#include <unrealsdk/unreal/classes/properties/uboolproperty.h>
#include <unrealsdk/unreal/classes/properties/ustrproperty.h>
#include <unrealsdk/unreal/structs/fstring.h>
#include <unrealsdk/unreal/wrappers/gobjects.h>
#include <unrealsdk/version_error.h>

#include "unrealsdk/logging.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/alignment.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/wrappers/gnames.h"

#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/prop_traits.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING) \
    && defined(UNREALSDK_GAME_BL1)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

// - NOTE -
// Currently everything is in a single file mostly for testing purposes. The patterns that have
// been found are not minimal and may contain jump offsets. Additionally, the offsets are for the
// BL1 1.4.1 UDK version of the game. I did a quick scan for the ProccessEvent pattern in the Steam
// version and it did find the correct function however I didn't check anymore than that; other
// than to see if FFrame::Step was also inlined, it is.
//

namespace {

// ############################################################################//
//  | GMALLOC |
// ############################################################################//

struct FMalloc;

struct FMallocVFtable {
    void* exec;
    void*(__thiscall* u_malloc)(FMalloc* self, uint32_t len, uint32_t align);
    void*(__thiscall* u_realloc)(FMalloc* self, void* original, uint32_t len, uint32_t align);
    void*(__thiscall* u_free)(FMalloc* self, void* data);
};

struct FMalloc {
    FMallocVFtable* vftable;
};

FMalloc* gmalloc;

// - NOTE -
// This is *unique* to GMalloc for the 141 UDK however its not completely unique. Its just all
//  other usages are also GMalloc.
//
const constinit Pattern<20> GMALLOC_PATTERN{
    "8B0D {????????}"  // mov ecx,dword ptr ds:[1F73BB4]
    "8B01"             // mov eax,dword ptr ds:[ecx]
    "8B50 04"          // mov edx,dword ptr ds:[eax+4]
    "6A 08"            // push 8
    "68 E4000000"      // push E4
    "FFD2"             // call edx
};

GObjects gobjects_wrapper{};

const constinit Pattern<15> GOBJECTS_SIG{
    "8B0D {????????}"  // mov ecx,dword ptr ds:[1FB85A8]
    "8B04B1"           // mov eax,dword ptr ds:[ecx+esi*4]
    "8B50 0C"          // mov edx,dword ptr ds:[eax+C]
    "2158 08"          // and dword ptr ds:[eax+8],ebx
};

GNames gnames_wrapper{};

const constinit Pattern<15> GNAMES_SIG{
    "A1 {????????}"  // mov eax,dword ptr ds:[1FB8578]
    "8B0CB0"         // mov ecx,dword ptr ds:[eax+esi*4]
    "68 00100000"    // push 1000
    "6A 00"          // push 0
};

// ############################################################################//
//  | LOAD PACKAGE |
// ############################################################################//

using load_package_func = UObject* (*)(const UObject* outer, const wchar_t* name, uint32_t flags);
load_package_func load_package_ptr;

const constinit Pattern<34> LOAD_PACKAGE_PATTERN{
    "55"              // push ebp
    "8B EC"           // mov ebp,esp
    "6A FF"           // push FFFFFFFF
    "68 ????????"     // push borderlands.18ECDB0
    "64 A1 00000000"  // mov eax,dword ptr fs:[0]
    "50"              // push eax
    "83EC 28"         // sub esp,28
    "53 56 57"        // push ebx
    "A1 ????????"     // push esi
    "33C5"            // push edi
    "50"              // mov eax,dword ptr ds:[1F16980]
    "8D45 F4"         // xor eax,ebp
};

// ############################################################################//
//  | FNAME INIT |
// ############################################################################//

const constinit Pattern<34> FNAME_INIT_SIG{
    "6A FF"            // push FFFFFFFF
    "68 ????????"      // push borderlands.18EB45B
    "64A1 00000000"    // mov eax,dword ptr fs:[0]
    "50"               // push eax
    "81EC 980C0000"    // sub esp,C98
    "A1 ????????"      // mov eax,dword ptr ds:[1F16980]
    "33C4"             // xor eax,esp
    "898424 940C0000"  // mov dword ptr ss:[esp+C94],eax
};

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__thiscall* fname_init_func)(FName* name,
                                          const wchar_t* str,
                                          int32_t number,
                                          int32_t find_type,
                                          int32_t split_name);

fname_init_func fname_init_ptr = nullptr;

// ############################################################################//
//  | FFRAME STEP |
// ############################################################################//

// - NOTE -
// This is inlined so we have to manually re-implement FFrame::Step using GNatives.
const constinit Pattern<36> FFRAME_STEP_SIG{
    "74 ??"              // je borderlands.59CEA4
    "8B45 D4"            // mov eax,dword ptr ss:[ebp-2C]
    "830D ???????? 02"   // or dword ptr ds:[1F73BE8],2
    "0FB610"             // movzx edx,byte ptr ds:[eax]
    "8B1495 {????????}"  // mov edx,dword ptr ds:[edx*4+1F97A80]
    "57"                 // push edi
    "8D4D BC"            // lea ecx,dword ptr ss:[ebp-44]
    "40"                 // inc eax
    "51"                 // push ecx
    "8B4D EC"            // mov ecx,dword ptr ss:[ebp-14]
    "8945 D4"            // mov dword ptr ss:[ebp-2C],eax
    "FFD2"               // call edx
};

// RET 0x8; Callee cleans up the stack (8 bytes)
// NOLINTNEXTLINE(modernize-use-using)
typedef void(__stdcall* fframe_step_func)(FFrame*, void*);
fframe_step_func** fframe_step_gnatives;

// ############################################################################//
//  | GET PATH NAME |
// ############################################################################//

// This one was basically identical to BL2.
// pattern: 8B4424 04 56 8BF1 3BF0 7476 85F6 7472 8B4E 28 53
typedef void(__thiscall* get_path_name_func)(const UObject* self,
                                             const UObject* stop_outer,
                                             ManagedFString* str);
get_path_name_func get_path_name_ptr;
const constinit Pattern<19> GET_PATH_NAME_PATTERN{
    "8B4424 04"  // mov eax,dword ptr ss:[esp+4]
    "56"         // push esi
    "8BF1"       // mov esi,ecx
    "3BF0"       // cmp esi,eax
    "74 76"      // je borderlands.5CF501
    "85F6"       // test esi,esi
    "74 72"      // je borderlands.5CF501
    "8B4E 28"    // mov ecx,dword ptr ds:[esi+28]
    "53"         // push ebx
};

// ############################################################################//
//  | CONSTRUCT OBJECT |
// ############################################################################//

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject*(__cdecl* construct_obj_func)(UClass* cls,
                                              UObject* outer,
                                              FName name,
                                              uint64_t flags,
                                              UObject* template_obj,
                                              void* error_output_device,
                                              void* instance_graph,
                                              uint32_t assume_template_is_archetype);
construct_obj_func construct_obj_ptr;

const constinit Pattern<49> CONSTRUCT_OBJECT_PATTERN{
    "6A FF"           // push FFFFFFFF
    "68 ????????"     // push borderlands.18ED693
    "64A1 00000000"   // mov eax,dword ptr fs:[0]
    "50"              // push eax
    "83EC 0C"         // sub esp,C
    "53"              // push ebx
    "55"              // push ebp
    "56"              // push esi
    "57"              // push edi
    "A1 ????????"     // mov eax,dword ptr ds:[1F16980]
    "33C4"            // xor eax,esp
    "50"              // push eax
    "8D4424 20"       // lea eax,dword ptr ss:[esp+20]
    "64 A3 00000000"  // mov dword ptr fs:[0],eax
    "8B6C24 54"       // mov ebp,dword ptr ss:[esp+54]
    "896C24 14"       // mov dword ptr ss:[esp+14],ebp
    "85ED"            // test ebp,ebp
};

// ############################################################################//
//  | STATIC FIND OBJECT |
// ############################################################################//

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject*(__cdecl* static_find_object_func)(const UClass* cls,
                                                   const UObject* package,
                                                   const wchar_t* str,
                                                   uint32_t exact_class);

static_find_object_func static_find_object_ptr;
const constinit Pattern<50> STATIC_FIND_OBJECT_PATTERN{
    "6A FF"          // push FFFFFFFF
    "68 ????????"    // push borderlands.18EC190
    "64A1 00000000"  // mov eax,dword ptr fs:[0]
    "50"             // push eax
    "83EC 24"        // sub esp,24
    "53"             // push ebx
    "55"             // push ebp
    "56"             // push esi
    "57"             // push edi
    "A1 ????????"    // mov eax,dword ptr ds:[1F16980]
    "33 C4"          // xor eax,esp
    "50"             // push eax
    "8D4424 38"      // lea eax,dword ptr ss:[esp+38]
    "64A3 00000000"  // mov dword ptr fs:[0],eax
    "8B7424 4C"      // mov esi,dword ptr ss:[esp+4C]
    "8B7C24 50"      // mov edi,dword ptr ss:[esp+50]
    "8BC6"           // mov eax,esi
    "40"             // inc eax
};

// ############################################################################//
//  | PROCESS EVENT |
// ############################################################################//

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* process_event_func)(UObject* obj,
                                             void* /*edx*/,
                                             UFunction* func,
                                             void* params,
                                             void* /*null*/);
process_event_func process_event_ptr;

const constinit Pattern<43> PROCESS_EVENT_SIG{
    "55"             // push ebp
    "8BEC"           // mov ebp,esp
    "6A FF"          // push FFFFFFFF
    "68 ????????"    // push <borderlands.sub_18E9668>
    "64A1 00000000"  // mov eax,dword ptr fs:[0]
    "50"             // push eax
    "83EC 40"        // sub esp,40
    "A1 ????????"    // mov eax,dword ptr ds:[1F16980]
    "33C5"           // xor eax,ebp
    "8945 F0"        // mov dword ptr ss:[ebp-10],eax
    "53"             // push ebx
    "56"             // push esi
    "57"             // push edi
    "50"             // push eax
    "8D45 F4"        // lea eax,dword ptr ss:[ebp-C]
    "64A3 00000000"  // mov dword ptr fs:[0],eax
};

// ############################################################################//
//  | CALL FUNCTION |
// ############################################################################//

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__fastcall* call_function_func)(UObject* obj,
                                             void* /*edx*/,
                                             FFrame* stack,
                                             void* params,
                                             UFunction* func);
call_function_func call_function_ptr;

const constinit Pattern<31> CALL_FUNCTION_SIG{
    "55"               // push ebp
    "8DAC24 FCFBFFFF"  // lea ebp,dword ptr ss:[esp-404]
    "81EC 04040000"    // sub esp,404
    "6A FF"            // push FFFFFFFF
    "68 ????????"      // push borderlands.18E9638
    "64 A1 00000000"   // mov eax,dword ptr fs:[0]
    "50"               // push eax
    "83EC 40"          // sub esp,40
};

// ############################################################################//
//  | SAVE PACKAGE FUNCTION |
// ############################################################################//

// - NOTE -
// This is only for the editor it might be useful much later on but right now I will just leave it
//  here so that it is known.
//
// The parameters into the function are not guaranteed to be correct I have verified that the
//  UObjects are valid though. Unknown_XY are bitwise anded with constants as well.
//

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject* (*save_package)(UObject* InOuter,
                                 UObject* InPackage,
                                 wchar_t* Filename,
                                 UObject* InTemplate,
                                 uint32_t Unknown_00,
                                 uint32_t Unknown_01,
                                 uint32_t Unknown_02,
                                 void* ErrorMessage,
                                 wchar_t* FileExtension,
                                 int32_t bSaveAsBinary,
                                 int32_t bOnlyIfDirty);

save_package save_package_ptr = nullptr;

const constinit Pattern<80> SAVE_PACKAGE_SIG{
    "6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 81 EC 30 09 00 00 A1 ?? ?? ?? ?? 33 C4 89 84 24 2C"
    " 09 00 00 53 55 56 57 A1 ?? ?? ?? ?? 33 C4 50 8D 84 24 44 09 00 00 64 A3 00 00 00 00 8B 84 24"
    " 6C 09 00 00 8B BC 24 54 09 00 00 8B AC 24 58 09 00 00"};

}  // namespace

// ############################################################################//
//  | BL1Hook |
// ############################################################################//

namespace unrealsdk::game {

void BL1Hook::hook(void) {
    hook_antidebug();

    hook_process_event();
    hook_call_function();
    // hook_save_package();

    find_gobjects();
    find_gnames();
    find_fname_init();
    find_fframe_step();
    find_gmalloc();
    find_construct_object();
    find_get_path_name();
    find_static_find_object();
    find_load_package();

    // These still need to be validated.
    hexedit_set_command();
    hexedit_array_limit();
    hexedit_array_limit_message();
}

namespace {

bool Hook_InstantlyLoadProfile(hook_manager::Details& in) {
    // bIsProfileLoaded is set to true after 30 seconds; This sets it to true once the warp-tunnel
    //  has finished.
    in.obj->get<UFunction, BoundFunction>(L"ClientSetProfileLoaded"_fn).call<void>();
    return false;
}

}  // namespace

void BL1Hook::post_init(void) {
    LOG(MISC, "Attaching Hooks!");
    inject_console();

    hook_manager::add_hook(L"WillowGame.WillowPlayerController:SpawningProcessComplete",
                           hook_manager::Type::POST, L"Hook_InstantlyLoadProfile",
                           &Hook_InstantlyLoadProfile);
}

// ############################################################################//
//  | FIND METHODS | CALLED IN BL1Hook::hook()
// ############################################################################//

void BL1Hook::find_gobjects(void) {
    auto gobjects_ptr = read_offset<GObjects::internal_type>(GOBJECTS_SIG.sigscan_nullable());
    LOG(MISC, "GObjects: {:p}", reinterpret_cast<void*>(gobjects_ptr));
    gobjects_wrapper = GObjects(gobjects_ptr);
}

void BL1Hook::find_gnames(void) {
    auto gnames_ptr = read_offset<GNames::internal_type>(GNAMES_SIG.sigscan_nullable());
    LOG(MISC, "GNames: {:p}", reinterpret_cast<void*>(gnames_ptr));
    gnames_wrapper = GNames(gnames_ptr);
}

void BL1Hook::find_fname_init(void) {
    fname_init_ptr = FNAME_INIT_SIG.sigscan_nullable<fname_init_func>();
    LOG(MISC, "FName::Init: {:p}", (void*)fname_init_ptr);
}

void BL1Hook::find_fframe_step(void) {
    fframe_step_gnatives = FFRAME_STEP_SIG.sigscan_nullable<fframe_step_func**>();

    if (fframe_step_gnatives == nullptr) {
        LOG(MISC, "FFrame::Step(...), GNatives was null.");
        return;
    }

    LOG(MISC, "FFrame::Step: {:p}", reinterpret_cast<void*>(fframe_step_gnatives));
}

void BL1Hook::find_gmalloc(void) {
    gmalloc = *read_offset<FMalloc**>(GMALLOC_PATTERN.sigscan("GMalloc"));
    LOG(MISC, "GMalloc: {:p}", reinterpret_cast<void*>(gmalloc));
}

void BL1Hook::find_construct_object(void) {
    construct_obj_ptr = CONSTRUCT_OBJECT_PATTERN.sigscan_nullable<construct_obj_func>();
    LOG(MISC, "StaticConstructObject: {:p}", reinterpret_cast<void*>(construct_obj_ptr));
}

namespace {
void __fastcall Hook_PathName(UObject* obj, void*, UObject* outer, ManagedFString* fstr) {
    LOG(MISC, "[Hook_PathName] ~ {:p}, {:p}, {:p}", (void*)obj, (void*)outer, (void*)fstr);
    get_path_name_ptr(obj, outer, fstr);
}
}  // namespace

void BL1Hook::find_get_path_name(void) {
    get_path_name_ptr = GET_PATH_NAME_PATTERN.sigscan<get_path_name_func>("get_path_name_func");
    LOG(MISC, "GetPathName: {:p}", reinterpret_cast<void*>(get_path_name_ptr));
}

void BL1Hook::find_static_find_object(void) {
    static_find_object_ptr = STATIC_FIND_OBJECT_PATTERN.sigscan_nullable<static_find_object_func>();
    LOG(MISC, "StaticFindObject: {:p}", reinterpret_cast<void*>(static_find_object_ptr));
}

namespace {

UObject* Hook_LoadPackage(const UObject* obj, const wchar_t* pkg, uint32_t flags) {
    std::wstring str{pkg};
    UObject* result = load_package_ptr(obj, pkg, flags);
    LOG(MISC, "[LOAD_PACKAGE] ~ {:p}, '{}', {:#06x}, {}", (void*)obj, str, flags,
        result->get_path_name());
    return result;
}
}  // namespace

void BL1Hook::find_load_package(void) {
    load_package_ptr = LOAD_PACKAGE_PATTERN.sigscan_nullable<load_package_func>();
    LOG(MISC, "LoadPackage: {:p}", reinterpret_cast<void*>(load_package_ptr));
}

// ############################################################################//
//  | PROCESS EVENT |
// ############################################################################//

void __fastcall Hook_ProcessEvent(UObject* obj,
                                  void* edx,
                                  UFunction* func,
                                  void* params,
                                  void* null) {
    try {
        // This arg seems to be in the process of being deprecated, no usage in ghidra, always seems
        // to be null, and it's gone in later ue versions. Gathering some extra info just in case.
        if (null != nullptr) {
            LOG(DEV_WARNING, L"Null param had a value in process event during func {} on obj {}",
                func->get_path_name(), obj->get_path_name());
        }

        auto data = hook_manager::impl::preprocess_hook(L"ProcessEvent", func, obj);
        if (data != nullptr) {
            // Copy args so that hooks can't modify them, for parity with call function
            const WrappedStruct args_base{func, params};
            WrappedStruct args = args_base.copy_params_only();
            hook_manager::Details hook{obj, &args, {func->find_return_param()}, {func, obj}};

            const bool block_execution =
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::PRE, hook);

            if (!block_execution) {
                process_event_ptr(obj, edx, func, params, null);
            }

            if (hook.ret.has_value()) {
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(params));
            }

            if (!hook_manager::impl::has_post_hooks(*data)) {
                return;
            }

            if (hook.ret.prop != nullptr && !hook.ret.has_value() && !block_execution) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(params));
            }

            if (!block_execution) {
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST, hook);
            }

            hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST_UNCONDITIONAL,
                                                  hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the ProcessEvent hook: {}", ex.what());
    }

    process_event_ptr(obj, edx, func, params, null);
}

void BL1Hook::hook_process_event(void) {
    detour(PROCESS_EVENT_SIG, &Hook_ProcessEvent, &process_event_ptr, "ProcessEvent");
}

// ############################################################################//
//  | CALL FUNCTION |
// ############################################################################//

void __fastcall Hook_CallFunction(UObject* obj,
                                  void* edx,
                                  FFrame* stack,
                                  void* result,
                                  UFunction* func) {
    try {
        auto data = hook_manager::impl::preprocess_hook(L"CallFunction", func, obj);
        if (data != nullptr) {
            WrappedStruct args{func};
            auto original_code = stack->extract_current_args(args);

            hook_manager::Details hook{obj, &args, {func->find_return_param()}, {func, obj}};

            const bool block_execution =
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::PRE, hook);

            if (block_execution) {
                stack->Code++;
            } else {
                stack->Code = original_code;
                call_function_ptr(obj, edx, stack, result, func);
            }

            if (hook.ret.has_value()) {
                // Result is a pointer directly to where the property should go, remove the offset
                hook.ret.copy_to(reinterpret_cast<uintptr_t>(result)
                                 - hook.ret.prop->Offset_Internal);
            }

            if (!hook_manager::impl::has_post_hooks(*data)) {
                return;
            }

            if (hook.ret.prop != nullptr && !hook.ret.has_value() && !block_execution) {
                hook.ret.copy_from(reinterpret_cast<uintptr_t>(result)
                                   - hook.ret.prop->Offset_Internal);
            }

            if (!block_execution) {
                hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST, hook);
            }

            hook_manager::impl::run_hooks_of_type(*data, hook_manager::Type::POST_UNCONDITIONAL,
                                                  hook);

            return;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR, "An exception occurred during the CallFunction hook: {}", ex.what());
    }

    call_function_ptr(obj, edx, stack, result, func);
}

void BL1Hook::hook_call_function(void) {
    detour(CALL_FUNCTION_SIG, &Hook_CallFunction, &call_function_ptr, "CallFunction");
}

// ############################################################################//
//  | SAVE PACKAGE |
// ############################################################################//

// NOTE: Leaving this here for now this is only relevant to the editor however I am unsure on the
//  input parameters. Even those named are not guaranteed.

namespace {
UObject* Hook_SavePackage(UObject* InOuter,
                          UObject* InPackage,
                          wchar_t* Filename,
                          UObject* InTemplate,
                          uint32_t Flags_00,
                          uint32_t Flags_01,
                          uint32_t Unknown_02,
                          void* ErrorMessage,
                          wchar_t* FileExtension,
                          int32_t bSaveAsBinary,
                          int32_t bOnlyIfDirty) {
    LOG(MISC, "[SAVE_PACKAGE] ~ {:p}, {:p}, {:p}, {:p}", (void*)InOuter, (void*)InPackage,
        (void*)Filename, (void*)InTemplate);

    UObject* result =
        save_package_ptr(InOuter, InPackage, Filename, InTemplate, Flags_00, Flags_01, Unknown_02,
                         ErrorMessage, FileExtension, bSaveAsBinary, bOnlyIfDirty);

    return result;
}
}  // namespace

void BL1Hook::hook_save_package(void) {
    detour(SAVE_PACKAGE_SIG, &Hook_SavePackage, &save_package_ptr, "Save Package");
}

// ############################################################################//
//  | BL2 IMPL SAYS NOT IMPLEMENTED |
// ############################################################################//

void BL1Hook::flazyobjectptr_assign(unreal::FLazyObjectPtr*, const unreal::UObject*) const {
    throw_version_error("Lazy object pointers are not implemented in UE3");
}

void BL1Hook::fsoftobjectptr_assign(unreal::FSoftObjectPtr*, const unreal::UObject*) const {
    throw_version_error("Lazy object pointers are not implemented in UE3");
}

void BL1Hook::ftext_as_culture_invariant(unreal::FText*, TemporaryFString&&) const {
    throw_version_error("Not implemented in UE3");
}

// ############################################################################//
//  | UE CORE STUFF |
// ############################################################################//

std::wstring BL1Hook::uobject_path_name(const unreal::UObject* obj) const {
    ManagedFString str{};
    get_path_name_ptr(obj, nullptr, &str);
    return str;
}

void BL1Hook::process_event(unreal::UObject* object, unreal::UFunction* func, void* params) const {
    Hook_ProcessEvent(object, nullptr, func, params, nullptr);
}

void BL1Hook::fframe_step(unreal::FFrame* frame, unreal::UObject*, void* param) const {
    uint8_t index = *frame->Code++;
    ((*fframe_step_gnatives)[index])(frame, param);
}

void BL1Hook::fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const {
    fname_init_ptr(name, str, number, 1, 1);
}

unreal::UObject* BL1Hook::load_package(const std::wstring& name, uint32_t flags) const {
    return load_package_ptr(nullptr, name.data(), flags);
}

unreal::UObject* BL1Hook::find_object(unreal::UClass* cls, const std::wstring& name) const {
    return static_find_object_ptr(cls, nullptr, name.c_str(), 0 /* false */);
}

UObject* BL1Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   decltype(UObject::ObjectFlags) flags,
                                   UObject* template_obj) const {
    return construct_obj_ptr(cls, outer, name, flags, template_obj, nullptr, nullptr, 0);
}

const unreal::GNames& BL1Hook::gnames(void) const {
    return gnames_wrapper;
}

const unreal::GObjects& BL1Hook::gobjects(void) const {
    return gobjects_wrapper;
}

void* BL1Hook::u_malloc(size_t len) const {
    void* ret = gmalloc->vftable->u_malloc(gmalloc, len, get_malloc_alignment(len));
    memset(ret, 0, len);
    return ret;
}

void* BL1Hook::u_realloc(void* original, size_t len) const {
    return gmalloc->vftable->u_realloc(gmalloc, original, len, get_malloc_alignment(len));
}

void BL1Hook::u_free(void* data) const {
    gmalloc->vftable->u_free(gmalloc, data);
}

// ############################################################################//
//  | HEX EDITS |
// ############################################################################//

namespace {

const constinit Pattern<17> SET_COMMAND_SIG{
    "75 16"        // jne borderlands.87E1A7
    "8D4C24 18"    // lea ecx,dword ptr ss:[esp+18]
    "68 ????????"  // push borderlands.1B18900
    "51"           // push ecx
    "E8 ????????"  // call <borderlands.sub_5C1E10>
};

}  // namespace

void BL1Hook::hexedit_set_command(void) {
    uint8_t* set_command_msg = SET_COMMAND_SIG.sigscan_nullable<uint8_t*>();

    if (set_command_msg == nullptr) {
        LOG(ERROR, "Failed to find set command message signature.");
        return;
    }

    LOG(INFO, "Set Command: {:p}", reinterpret_cast<void*>(set_command_msg));

    // NOLINTBEGIN(readability-magic-numbers)
    unlock_range(set_command_msg, 2);
    set_command_msg[0] = 0x90;
    set_command_msg[1] = 0x90;
    // NOLINTEND(readability-magic-numbers)
}

namespace {

const constinit Pattern<29> ARRAY_LIMIT_MESSAGE{
    "0F8C 7E000000"  // jl borderlands.5E7E64
    "8B4C24 38"      // mov ecx,dword ptr ss:[esp+38]
    "83C7 9D"        // add edi,FFFFFF9D
    "57"             // push edi
    "68 ????????"    // push borderlands.1A7F42C
    "E8 ????????"    // call borderlands.51BAA0
    "E9 ????????"    // jmp borderlands.5E7E64
};
}  // namespace

void BL1Hook::hexedit_array_limit_message(void) const {
    uint8_t* array_limit_msg = ARRAY_LIMIT_MESSAGE.sigscan_nullable<uint8_t*>();

    if (array_limit_msg == nullptr) {
        LOG(ERROR, "Failed to find array limit message signature.");
        return;
    }

    LOG(MISC, "Array Limit Message: {:p}", reinterpret_cast<void*>(array_limit_msg));

    // NOLINTBEGIN(readability-magic-numbers)
    unlock_range(array_limit_msg, 6);
    array_limit_msg[0] = 0xE9;
    array_limit_msg[1] = 0x7F;
    array_limit_msg[2] = 0x00;
    array_limit_msg[3] = 0x00;
    array_limit_msg[4] = 0x00;
    array_limit_msg[5] = 0x90;
    // NOLINTEND(readability-magic-numbers)
}

namespace {

// - NOTE -
// In BL2 this seems to be inlined however for BL1 its not so we will NOP the CALL and its 4 byte
// address. The caller cleans the stack so need to worry about that. Might also be a good idea to
// move the signature forward to the CALL so we don't need to index weirdly.
//

const constinit Pattern<20> ARRAY_LIMIT_SIG{
    "6A 64"          // push 64
    "50"             // push eax
    "46"             // inc esi
    "E8 ????????"    // call <borderlands.sub_517770>
    "83C4 08"        // add esp,8
    "3BF0"           // cmp esi,eax
    "0F8C 59FFFFFF"  // jl borderlands.5E7D33
};

}  // namespace

void BL1Hook::hexedit_array_limit(void) {
    auto array_limit = ARRAY_LIMIT_SIG.sigscan_nullable<uint8_t*>();
    if (array_limit == nullptr) {
        LOG(ERROR, "Couldn't find array limit signature");
    } else {
        LOG(MISC, "Array Limit: {:p}", reinterpret_cast<void*>(array_limit));

        // NOLINTBEGIN(readability-magic-numbers)
        auto* call_instruction = array_limit + 4;

        if (call_instruction[0] != 0xE8) {
            LOG(ERROR, "[ARRAY_LIMIT] ~ Instruction at {:p} + 0x04 is {:02X}", (void*)array_limit,
                *call_instruction);
            return;
        }

        // Patching out the CALL ?? ?? ?? ?? to 5 NOP instructions
        unlock_range(call_instruction, 5);
        call_instruction[0] = 0x90;
        call_instruction[1] = 0x90;
        call_instruction[2] = 0x90;
        call_instruction[3] = 0x90;
        call_instruction[4] = 0x90;
        // NOLINTEND(readability-magic-numbers)
    }
}

// ############################################################################//
//  | CONSOLE STUFF |
// ############################################################################//

namespace {

bool Hook_RemoveSayFromConsoleCommand(unrealsdk::hook_manager::Details& in) {
    static const auto console_command_func =
        in.obj->Class->find_func_and_validate(L"ConsoleCommand"_fn);

    static const auto command_property =
        in.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);

    auto param = in.args->get<UStrProperty>(command_property);
    auto func = in.obj->get<UFunction, BoundFunction>(console_command_func);
    func.call<void, UStrProperty>(param);

    return true;
}

BoundFunction s_ConsoleOutputText{};

bool Hook_InjectConsole(hook_manager::Details& hook) {
    hook_manager::remove_hook(L"WillowGame.WillowGameViewportClient:PostRender",
                              hook_manager::Type::PRE, L"Hook_InjectConsole");

    auto console = hook.obj->get<UObjectProperty>(L"ViewportConsole"_fn);
    LOG(MISC, "[Hook_InjectConsole] ~ {:p}, '{}'", (void*)console, console->get_path_name());
    s_ConsoleOutputText = console->get<UFunction, BoundFunction>(L"OutputTextLine"_fn);

    return false;
}

bool Hook_ConsoleCommandImpl(hook_manager::Details& hook) {
    const auto command_property =
        hook.args->type->find_prop_and_validate<UStrProperty>(L"Command"_fn);

    const auto history_prop = hook.obj->Class->find_prop_and_validate<UStrProperty>(L"History"_fn);
    const auto history_top_prop =
        hook.obj->Class->find_prop_and_validate<UIntProperty>(L"HistoryTop"_fn);
    const auto history_bot_prop =
        hook.obj->Class->find_prop_and_validate<UIntProperty>(L"HistoryBot"_fn);
    const auto history_cur_prop =
        hook.obj->Class->find_prop_and_validate<UIntProperty>(L"HistoryCur"_fn);

    const UFunction* purge_command_func =
        hook.obj->Class->find_func_and_validate(L"PurgeCommandFromHistory"_fn);
    const UFunction* save_config_func = hook.obj->Class->find_func_and_validate(L"SaveConfig"_fn);

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
}  // namespace

bool BL1Hook::is_console_ready(void) const {
    return s_ConsoleOutputText.func != nullptr;
}

void BL1Hook::uconsole_output_text(const std::wstring& str) const {
    if (!is_console_ready()) {
        return;
    }
    s_ConsoleOutputText.call<void, UStrProperty>(str);
}

void BL1Hook::inject_console(void) {
    // clang-format off
    hook_manager::add_hook(
        L"Engine.Console:ShippingConsoleCommand",
        unrealsdk::hook_manager::Type::PRE,
        L"Hook_RemoveSayFromConsoleCommand",
        &Hook_RemoveSayFromConsoleCommand
    );

    hook_manager::add_hook(
        L"Engine.Console:ConsoleCommand",
        hook_manager::Type::PRE,
        L"Hook_ConsoleCommandImpl",
        &Hook_ConsoleCommandImpl
    );

    hook_manager::add_hook(
        L"WillowGame.WillowGameViewportClient:PostRender",
        hook_manager::Type::PRE,
        L"Hook_InjectConsole",
        &Hook_InjectConsole
    );
    // clang-format on
}

}  // namespace unrealsdk::game

#endif