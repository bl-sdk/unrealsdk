#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl1/bl1.h"

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/version_error.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

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
}

void BL1Hook::post_init(void) {
    inject_console();
}

#pragma region FFrame::Step

namespace {

// FFrame::Step is inlined, so instead we manually re-implement it using GNatives.
const constinit Pattern<11> GNATIVES_SIG{
    "8B 14 95 {????????}"  // mov edx, [edx*4+01F942C0]
    "57"                   // push edi
    "8D 4D ??"             // lea ecx, [ebp-44]
};

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__thiscall* fframe_step_func)(UObject*, FFrame*, void*);
fframe_step_func** fframe_step_gnatives;

}  // namespace

void BL1Hook::find_fframe_step(void) {
    fframe_step_gnatives = GNATIVES_SIG.sigscan_nullable<fframe_step_func**>();
    LOG(MISC, "GNatives: {:p}", reinterpret_cast<void*>(fframe_step_gnatives));
}

void BL1Hook::fframe_step(FFrame* frame, UObject* obj, void* param) const {
    ((*fframe_step_gnatives)[*frame->Code++])(obj, frame, param);
}

#pragma region FName::Init

namespace {

#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"  // thiscall on non-class
#endif

const constinit Pattern<72> FNAME_INIT_SIG{
    "6A FF"              // push -01
    "68 ????????"        // push 018E73CB
    "64 A1 ????????"     // mov eax, fs:[00000000]
    "50"                 // push eax
    "81 EC ????????"     // sub esp, 00000C98
    "A1 ????????"        // mov eax, [01F131C0]
    "33 C4"              // xor eax, esp
    "89 84 24 ????????"  // mov [esp+00000C94], eax
    "53"                 // push ebx
    "55"                 // push ebp
    "56"                 // push esi
    "57"                 // push edi
    "A1 ????????"        // mov eax, [01F131C0]
    "33 C4"              // xor eax, esp
    "50"                 // push eax
    "8D 84 24 ????????"  // lea eax, [esp+00000CAC]
    "64 A3 ????????"     // mov fs:[00000000], eax
    "8B BC 24 ????????"  // mov edi, [esp+00000CBC]
    "8B E9"              // mov ebp, ecx
    "89 6C 24 ??"        // mov [esp+1C], ebp
};

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__thiscall* fname_init_func)(FName* name,
                                          const wchar_t* str,
                                          int32_t number,
                                          int32_t find_type,
                                          int32_t split_name);

fname_init_func fname_init_ptr = nullptr;

#if defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

}  // namespace

void BL1Hook::find_fname_init(void) {
    fname_init_ptr = FNAME_INIT_SIG.sigscan_nullable<fname_init_func>();
    LOG(MISC, "FName::Init: {:p}", (void*)fname_init_ptr);
}

void BL1Hook::fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const {
    fname_init_ptr(name, str, number, 1, 1);
}

#pragma endregion

#pragma region FText::AsCultureInvariant

void BL1Hook::ftext_as_culture_invariant(unreal::FText* /*text*/,
                                         TemporaryFString&& /*str*/) const {
    throw_version_error("FTexts are not implemented in UE3");
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
