#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl1/bl1.h"

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/version_error.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING) \
    && defined(UNREALSDK_GAME_BL1)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

// - NOTE -
// The offsets are for the BL1 1.4.1 UDK version of the game. I did a quick scan for the
// ProccessEvent pattern in the Steam version and it did find the correct function however I
// didn't check anymore than that; other than to see if FFrame::Step was also inlined, it is.
//
// I have done no work and put no effort into seeing if anything here can be used for the Enhanced
// version of the game.
//

namespace unrealsdk::game {

// These could be defined in the class but since they are only used here this will do for now.
namespace {
void hook_save_package(void);
void inject_qol_hooks(void);
}  // namespace

void BL1Hook::hook(void) {
    hook_antidebug();

    hook_process_event();
    hook_call_function();

    find_gobjects();
    find_gnames();
    find_fname_init();
    find_fframe_step();
    find_gmalloc();
    find_construct_object();
    find_get_path_name();
    find_static_find_object();
    find_load_package();

    hexedit_set_command();
    hexedit_array_limit();
    hexedit_array_limit_message();
}

void BL1Hook::post_init(void) {
    LOG(MISC, "Attaching Hooks!");
    inject_console();
    inject_qol_hooks();
}

// ############################################################################//
//  | FFRAME STEP |
// ############################################################################//

namespace {

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

}  // namespace

void BL1Hook::find_fframe_step(void) {
    fframe_step_gnatives = FFRAME_STEP_SIG.sigscan_nullable<fframe_step_func**>();

    if (fframe_step_gnatives == nullptr) {
        LOG(ERROR, "FFrame::Step(...), GNatives was null.");
        return;
    }

    LOG(MISC, "FFrame::Step: {:p}", reinterpret_cast<void*>(fframe_step_gnatives));
}

void BL1Hook::fframe_step(unreal::FFrame* frame, unreal::UObject*, void* param) const {
    ((*fframe_step_gnatives)[*frame->Code++])(frame, param);
}

// ############################################################################//
//  | FNAME INIT |
// ############################################################################//

namespace {

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

}  // namespace

void BL1Hook::find_fname_init(void) {
    fname_init_ptr = FNAME_INIT_SIG.sigscan_nullable<fname_init_func>();
    LOG(MISC, "FName::Init: {:p}", (void*)fname_init_ptr);
}

void BL1Hook::fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const {
    fname_init_ptr(name, str, number, 1, 1);
}

// ############################################################################//
//  | SAVE PACKAGE |
// ############################################################################//

namespace {

// - NOTE -
// This is an function editor it might be useful much later on but right now I will leave it here
//  so that it is known.
//
// The parameters into the function are not guaranteed to be correct I have verified that the
//  UObjects are valid though. Unknown_XY are bitwise anded with constants as well.
//

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject* (*save_package_func)(UObject* InOuter,
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

save_package_func save_package_ptr = nullptr;

const constinit Pattern<80> SAVE_PACKAGE_SIG{
    "6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 81 EC 30 09 00 00 A1 ?? ?? ?? ?? 33 C4 89 84 24 2C"
    " 09 00 00 53 55 56 57 A1 ?? ?? ?? ?? 33 C4 50 8D 84 24 44 09 00 00 64 A3 00 00 00 00 8B 84 24"
    " 6C 09 00 00 8B BC 24 54 09 00 00 8B AC 24 58 09 00 00"};

UObject* hook_save_package_detour(UObject* InOuter,
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

void hook_save_package(void) {
    detour(SAVE_PACKAGE_SIG, &hook_save_package_detour, &save_package_ptr, "bl1_hook_save_package");
}

}  // namespace

// ############################################################################//
//  | CUSTOM HOOKS |
// ############################################################################//

namespace {

bool hook_instantly_load_profile(hook_manager::Details& in) {
    // bIsProfileLoaded is set to true after 30 seconds; This sets it to true once the warp-tunnel
    //  has finished. This allows you to instantly save on quit rather than having to wait 30s.
    in.obj->get<UFunction, BoundFunction>(L"ClientSetProfileLoaded"_fn).call<void>();
    return false;
}

void inject_qol_hooks(void) {
    hook_manager::add_hook(L"WillowGame.WillowPlayerController:SpawningProcessComplete",
                           hook_manager::Type::POST, L"bl1_hook_instantly_load_profile",
                           &hook_instantly_load_profile);
}

}  // namespace

// ############################################################################//
//  | NOT IMPLEMENTED |
// ############################################################################//

void BL1Hook::ftext_as_culture_invariant(unreal::FText*, TemporaryFString&&) const {
    throw_version_error("FTexts are not implemented in UE3");
}

}  // namespace unrealsdk::game

#endif