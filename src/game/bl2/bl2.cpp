#include "pch.h"

#include "game/bl2/bl2.h"
#include "memory.h"
#include "unreal/classes/uclass.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/structs/fname.h"

#if defined(UE3) && defined(ARCH_X86)

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

    inject_console();

    hexedit_set_command();
    hexedit_array_limit();
    hexedit_array_limit_message();
}

#pragma region FName::Init

void BL2Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_SIG{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x81\xEC\x9C\x0C",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    this->fname_init_ptr = sigscan<void*>(FNAME_INIT_SIG);
    LOG(MISC, "FName::Init: {:p}", this->fname_init_ptr);
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

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__thiscall* fframe_step_func)(FFrame* stack, UObject* obj, void* param);
static fframe_step_func fframe_step_ptr;

void BL2Hook::find_fframe_step(void) {
    static const Pattern FFRAME_STEP_SIG{"\x55\x8B\xEC\x8B\x41\x18\x0F\xB6\x10",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    fframe_step_ptr = sigscan<fframe_step_func>(FFRAME_STEP_SIG);
    LOG(MISC, "FFrame::Step: {:p}", reinterpret_cast<void*>(fframe_step_ptr));
}
void BL2Hook::fframe_step(FFrame* frame, UObject* obj, void* param) const {
    fframe_step_ptr(frame, obj, param);
}

#pragma endregion

#pragma region ConstructObject

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject*(__cdecl* construct_obj_func)(UClass* cls,
                                              UObject* outer,
                                              FName name,
                                              uint64_t flags,
                                              UObject* template_obj,
                                              void* error_output_device,
                                              void* instance_graph,
                                              uint32_t assume_template_is_archetype);
static construct_obj_func construct_obj_ptr;

void BL2Hook::find_construct_object(void) {
    static const Pattern CONSTRUCT_OBJECT_PATTERN{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x10\x53\x56"
        "\x57\xA1\x00\x00\x00\x00\x33\xC5\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\x7D\x08\x8A"
        "\x87",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF"
        "\xFF"};
    construct_obj_ptr = sigscan<construct_obj_func>(CONSTRUCT_OBJECT_PATTERN);
    LOG(MISC, "StaticConstructObject: {:p}", reinterpret_cast<void*>(construct_obj_ptr));
}

UObject* BL2Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   decltype(UObject::ObjectFlags) flags,
                                   UObject* template_obj) const {
    return construct_obj_ptr(cls, outer, name, flags, template_obj, nullptr, nullptr,
                             0 /* false */);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
