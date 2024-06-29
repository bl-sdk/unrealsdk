#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/version_error.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

void BL2Hook::hook(void) {
    // Make sure to do antidebug asap
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

void BL2Hook::post_init(void) {
    inject_console();
}

#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"  // thiscall on non-class
#endif

#pragma region FName::Init

namespace {

const constinit Pattern<23> FNAME_INIT_SIG{
    "55"              // push ebp
    "8B EC"           // mov ebp, esp
    "6A FF"           // push -01
    "68 ????????"     // push Borderlands2.exe+110298B
    "64 A1 ????????"  // mov eax, fs:[00000000]
    "50"              // push eax
    "81 EC 9C0C0000"  // sub esp, 00000C9C
};

}

void BL2Hook::find_fname_init(void) {
    this->fname_init_ptr = FNAME_INIT_SIG.sigscan<void*>();
    LOG(MISC, "FName::Init: {:p}", this->fname_init_ptr);
}

void BL2Hook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    // NOLINTNEXTLINE(modernize-use-using)  - need a typedef for the __thiscall
    typedef void*(__thiscall * fname_init_func)(FName * name, const wchar_t* str, int32_t number,
                                                int32_t find_type, int32_t split_name);

    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str, number, 1, 1);
}

#pragma endregion

#pragma region FFrame::Step

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__thiscall* fframe_step_func)(FFrame* stack, UObject* obj, void* param);
fframe_step_func fframe_step_ptr;

const constinit Pattern<9> FFRAME_STEP_SIG{
    "55"        // push ebp
    "8B EC"     // mov ebp, esp
    "8B 41 ??"  // mov eax, [ecx+18]
    "0FB6 10"   // movzx edx, byte ptr [eax]
};

}  // namespace

void BL2Hook::find_fframe_step(void) {
    fframe_step_ptr = FFRAME_STEP_SIG.sigscan<fframe_step_func>();
    LOG(MISC, "FFrame::Step: {:p}", reinterpret_cast<void*>(fframe_step_ptr));
}
void BL2Hook::fframe_step(FFrame* frame, UObject* obj, void* param) const {
    fframe_step_ptr(frame, obj, param);
}

#pragma endregion

#pragma region FText::AsCultureInvariant

void BL2Hook::ftext_as_culture_invariant(unreal::FText* /*text*/,
                                         unreal::TemporaryFString&& /*str*/) const {
    throw_version_error("FTexts are not implemented in UE3");
}

#pragma endregion

#if defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

}  // namespace unrealsdk::game

#endif
