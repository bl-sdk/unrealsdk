#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl1e/bl1e.h"

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/version_error.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

void BL1EHook::hook(void) {
    wait_for_steam_drm();

    // do this asap since it messes with early launch functions
    try {
        hexedit_editor_access();
    } catch (const std::exception& err) {
        LOG(WARNING, "Failed to unlock the editor ~ {}", err.what());
    }

    hook_process_event();
    hook_call_function();

    find_gmalloc();
    find_gobjects();
    find_gnames();
    find_fname_init();
    find_fframe_step();
    find_construct_object();
    find_get_path_name();
    find_static_find_object();
    find_load_package();
}

void BL1EHook::post_init(void) {
    inject_console();
}

namespace {

// 40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC E0 0C 00 00
constexpr Pattern<19> FNAME_INIT_SIG{
    "40 55"              // PUSH  RBP
    "56"                 // PUSH  RSI
    "57"                 // PUSH  RDI
    "41 54"              // PUSH  R12
    "41 55"              // PUSH  R13
    "41 56"              // PUSH  R14
    "41 57"              // PUSH  R15
    "48 81 EC E00C0000"  // SUB   RSP,0xCE0
};

constexpr Pattern<33> GNATIVES_SIG{
    "33 C9"                // XOR     ECX,ECX
    "48 39 15 ????????"    // CMP     qword ptr [BL1E_GNatives]
    "0F 45 C1"             // CMOVNZ  EAX,ECX
    "89 05 ????????"       // MOV     dword ptr [DAT_142546260],EAX
    "48 8B 05 ????????"    // MOV     RAX=>FUN_140144f50,qword ptr
    "48 89 05 {????????}"  // MOV     qword ptr [BL1E_GNatives]
    "C3"                   // RET
};

using native_func = void (UObject::*)(FFrame* stack, void* result);
native_func* fframe_step_gnatives{nullptr};

}  // namespace

void BL1EHook::find_fframe_step(void) {
    fframe_step_gnatives = read_offset<native_func*>(GNATIVES_SIG.sigscan_nullable());
    LOG(MISC, "GNatives: {:p}", reinterpret_cast<void*>(fframe_step_gnatives));
}

void BL1EHook::fframe_step(FFrame* frame, UObject* obj, void* param) const {
    auto curr_native = *frame->Code();
    frame->Code()++;
    (obj->*fframe_step_gnatives[curr_native])(frame, param);
}

namespace {

using fname_init_func = void (*)(FName* name,
                                 const wchar_t* str,
                                 uint32_t number,
                                 int32_t find_type,
                                 int32_t split_name);

fname_init_func fname_init_ptr = nullptr;

}  // namespace

void BL1EHook::find_fname_init(void) {
    fname_init_ptr = FNAME_INIT_SIG.sigscan_nullable<fname_init_func>();
    LOG(MISC, "FName::Init: {:p}", reinterpret_cast<void*>(fname_init_ptr));
}

void BL1EHook::fname_init(FName* name, const wchar_t* str, uint32_t number) const {
    fname_init_ptr(name, str, number, 1, 1);
}

}  // namespace unrealsdk::game

#endif
