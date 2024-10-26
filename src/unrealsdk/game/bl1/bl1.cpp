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
static std::atomic_bool bl1_has_initialised{false};

void hook_save_package(void);
void hook_resolve_error(void);
void hook_init_func(void);
}  // namespace

void BL1Hook::hook(void) {
    hook_antidebug();

    if (!env::defined(KEY_DO_NOT_WAIT_FOR_INIT)) {
        hook_init_func();
        // Shouldn't take longer than 60s tbh but just incase someones pc is exceptionally slow.
        const float MAX_WAIT_TIME = env::get_numeric(KEY_MAX_WAIT_TIME, 120.0F);
        using Clock = std::chrono::steady_clock;
        auto start = Clock::now();

        // Wait until the game has initialised or until we timeout
        while (!bl1_has_initialised.load(std::memory_order_relaxed)) {
            // yielding is an option but this is probably better
            std::this_thread::sleep_for(std::chrono::milliseconds{100});

            float elapsed = std::chrono::duration<float>(Clock::now() - start).count();
            if (elapsed > MAX_WAIT_TIME) {
                LOG(INFO,
                    "bl1sdk is aborting initialisation as it has taken too long to initialise.");
                return;
            }
        }
    }

    hook_process_event();
    hook_call_function();

    if (env::defined(KEY_LOG_SAVE_PKG)) {
        hook_save_package();
    }

    if (env::defined(KEY_LOG_EXTENDED_DBG)) {
        hook_resolve_error();
    }

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

// NOLINTNEXTLINE(modernize-use-using)
typedef int32_t (*save_package_func)(UObject* InOuter,
                                     UObject* Base,
                                     int64_t TopLevelFlags,
                                     wchar_t* Filename,
                                     void* Error,  // non-null
                                     void* Conform,
                                     bool bForceByteSwapping,
                                     bool bWarnOfLongFilename,
                                     uint32_t SaveFlags,
                                     UObject* TargetPlatform,  // ?
                                     void* FinalTimeStamp,
                                     int Unknown_00 /* pointer? */);

save_package_func save_package_ptr = nullptr;

const constinit Pattern<51> SAVE_PACKAGE_SIG{
    "55 8D AC 24 D4 F3 FF FF 81 EC 2C 0C 00 00 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 81 EC A0"
    "03 00 00 A1 ?? ?? ?? ?? 33 C5 89 85 28 0C 00 00 53 56 57 50"};

int32_t hook_save_package_detour(UObject* InOuter,
                                 UObject* Base,
                                 int64_t TopLevelFlags,
                                 wchar_t* Filename,
                                 void* Error,
                                 void* Conform,
                                 bool bForceByteSwapping,
                                 bool bWarnOfLongFilename,
                                 uint32_t SaveFlags,
                                 UObject* TargetPlatform,
                                 void* FinalTimeStamp,
                                 int Unknown_00) {
    LOG(MISC, L"Saving Package: {:p}, {:p}, {:#016x}, {:#08x}, '{}'",
        reinterpret_cast<void*>(InOuter), reinterpret_cast<void*>(Base), TopLevelFlags, SaveFlags,
        Filename);

    int32_t result = save_package_ptr(InOuter, Base, TopLevelFlags, Filename, Error, Conform,
                                      bForceByteSwapping, bWarnOfLongFilename, SaveFlags,
                                      TargetPlatform, FinalTimeStamp, Unknown_00);
    return result;
}

void hook_save_package(void) {
    detour(SAVE_PACKAGE_SIG, &hook_save_package_detour, &save_package_ptr, "bl1_hook_save_package");
}

}  // namespace

// ############################################################################//
//  | EXTENDED DEBUGGING |
// ############################################################################//

namespace {

// - NOTE -
// I've seen this function get called with static strings quite a fair bit in ghidra could be useful
// for identifying soft/critical errors.
//

const constinit Pattern<28> EXTENDED_DEBUGGING_SIG{
    "51 8B 44 24 14 8B 4C 24 10 8B 54 24 0C 56 8B 74 24 0C 6A 00 50 51 52 68 ?? ?? ?? ??"};

// NOLINTNEXTLINE(modernize-use-using)
typedef wchar_t**(__cdecl* resolve_error)(wchar_t**, wchar_t*, wchar_t*, int32_t);

resolve_error resolve_error_ptr = nullptr;

wchar_t** resolve_error_detour(wchar_t** obj, wchar_t* error, wchar_t* ctx, int32_t flags) {
    wchar_t** msg = resolve_error_ptr(obj, error, ctx, flags);
    // [RESOLVE_ERR] Core::ObjectNotFound | 0x0 'Object not found ...'
    // LOG(WARNING, L"[RESOLVE_ERR] {}::{} | {:#08x} '{}'", ctx, error, flags, *msg);
    return msg;
}

void hook_resolve_error(void) {
    detour(EXTENDED_DEBUGGING_SIG, &resolve_error_detour, &resolve_error_ptr,
           "bl1_hook_raise_error");
}

}  // namespace

// ############################################################################//
//  | CUSTOM HOOKS |
// ############################################################################//

namespace {

// - NOTE -
// The init function is only used to delay initialisation of the SDK to ensure that we can proceed
// with injection at the right time. However, the Steams version of this function is different
// the core functionality though is the same. So we will try the UDK version and then the Steam
// version. If both fail then we won't crash but it will be a 'Silent' error for most users which
// is not ideal.

const constinit Pattern<64> INIT_FUNC_STEAM_SIG{
    "6A FF"          // push FFFFFFFF
    "68 ????????"    // push borderlands.198D998
    "64A1 00000000"  // mov eax,dword ptr fs:[0]
    "50"             // push eax
    "83EC 3C"        // sub esp,3C
    "53"             // push ebx
    "55"             // push ebp
    "56"             // push esi
    "57"             // push edi
    "A1 ????????"    // mov eax,dword ptr ds:[1F131C0]
    "33C4"           // xor eax,esp
    "50"             // push eax
    "8D4424 50"      // lea eax,dword ptr ss:[esp+50]
    "64A3 00000000"  // mov dword ptr fs:[0],eax
    "8BD9"           // mov ebx,ecx
    "68 ????????"    // push borderlands.1CC3E78
    "E8 ????????"    // call borderlands.5C20F0
    "50"             // push eax
    "E8 ????????"    // call borderlands.5C2A80
    "83C4 08"        // add esp,8
    "85C0"           // test eax,eax
    "74 ??"          // je borderlands.138F06A
};

const constinit Pattern<45> INIT_FUNC_141_UDK_SIG{
    "6A FF"          // push FFFFFFFF
    "68 ????????"    // push <borderlands.sub_1991338>
    "64A1 00000000"  // mov eax,dword ptr fs:[0]
    "50"             // push eax
    "83EC 3C"        // sub esp,3C
    "53"             // push ebx
    "55"             // push ebp
    "56"             // push esi
    "57"             // push edi
    "A1 ????????"    // mov eax,dword ptr ds:[1F16980]
    "33C4"           // xor eax,esp
    "50"             // push eax
    "8D4424 50"      // lea eax,dword ptr ss:[esp+50]
    "64A3 00000000"  // mov dword ptr fs:[0],eax
    "8BD9"           // mov ebx,ecx
    "EB ??"          // jmp borderlands.13ADAC9
    "4D"             // dec ebp
    "61"             // popad
};

// Not sure if this is a __thiscall or an actual __fastcall; ecx is used.
typedef void(__fastcall* init_function)(void* ecx, void* edx);

init_function init_func_ptr = nullptr;

void __fastcall detour_init_func(void* ecx, void* edx) {
    init_func_ptr(ecx, edx);
    // When this is true the unrealscript game engine has been created
    bl1_has_initialised.store(true, std::memory_order_relaxed);
}

void hook_init_func(void) {
    // - NOTE -
    // This is like this because the Steam version has different loading logic which causes the
    // offsets to not be found. Now fundementally this is a problem of microseconds, as in, by
    // compiling in debug mode the hook will work all the time. So, the optimisations provided in
    // release mode are to cause the hooks to fail. So to stop this we will try to hook more
    // than once with a small delay between each attempt. We could disable optimisations here but
    // thats just stupid lol.
    //
    // This way also has the added benefit of working even if you launch the game from outside of
    // Steam however I still wouldn't recommend it.
    //
    constexpr size_t MAX_ATTEMPTS = 10;
    constexpr size_t DEFAULT_DELAY = 100;
    constexpr uintptr_t INVALID_ADDR{0};

    for (size_t i = 0; i < MAX_ATTEMPTS; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_DELAY));
        uintptr_t udk_addr = INIT_FUNC_141_UDK_SIG.sigscan_nullable();
        uintptr_t steam_addr = INIT_FUNC_STEAM_SIG.sigscan_nullable();

        // This should never happen but if it does we will want to know
        if (udk_addr != INVALID_ADDR && steam_addr != INVALID_ADDR) {
            LOG(ERROR,
                "Found the UDK init function signature and the Steam init function..."
                " That shouldn't be possible...");
            continue;
        }

        // Hook UDK
        if (udk_addr != INVALID_ADDR) {
            LOG(INFO, "Found UDK Init function at {:#016x} after {} attempts", udk_addr, i);
            detour(udk_addr, &detour_init_func, &init_func_ptr, "bl1_hook_init_func");
            return;
        }

        // Hook Steam
        if (steam_addr != INVALID_ADDR) {
            LOG(INFO, "Found Steam Init function at {:#016x} after {} attempts", steam_addr, i);
            detour(steam_addr, &detour_init_func, &init_func_ptr, "bl1_hook_init_func");
            return;
        }
    }
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