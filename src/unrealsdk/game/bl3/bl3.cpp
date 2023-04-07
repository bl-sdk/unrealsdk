#include "pch.h"

#include "game/bl3/bl3.h"
#include "memory.h"
#include "unreal/structs/fstring.h"
#include "unreal/structs/tarray.h"
#include "unrealsdk.h"

#if defined(UE4) && defined(ARCH_X64)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

void BL3Hook::hook(void) {
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

    inject_console();
}

#pragma region FName::Init

namespace {

using fname_init_func = void (*)(FName* self,
                                 const wchar_t* str,
                                 int32_t number,
                                 int32_t find_type,
                                 int32_t split_name,
                                 int32_t hardcode_idx);
fname_init_func fname_init_ptr;

}  // namespace

void BL3Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_PATTERN{
        "\x48\x89\x5C\x24\x00\x55\x56\x57\x48\x81\xEC\x60\x08\x00\x00",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    fname_init_ptr = sigscan<fname_init_func>(FNAME_INIT_PATTERN);
    LOG(MISC, "FName::Init: {:p}", reinterpret_cast<void*>(fname_init_ptr));
}

void BL3Hook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    fname_init_ptr(name, str, number, 1, 1 /* true */, -1);
}

#pragma endregion

#pragma region FFrame::Step

namespace {

using fframe_step_func = void (*)(FFrame* stack, UObject* obj, void* param);
fframe_step_func fframe_step_ptr;

}  // namespace

void BL3Hook::find_fframe_step(void) {
    static const Pattern FFRAME_STEP_SIG{"\x48\x8B\x41\x20\x4C\x8B\xD2\x48\x8B\xD1",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    fframe_step_ptr = sigscan<fframe_step_func>(FFRAME_STEP_SIG);
    LOG(MISC, "FFrame::Step: {:p}", reinterpret_cast<void*>(fframe_step_ptr));
}
void BL3Hook::fframe_step(FFrame* frame, UObject* obj, void* param) const {
    fframe_step_ptr(frame, obj, param);
}

#pragma endregion

#pragma region ConstructObject

namespace {

using construct_obj_func = UObject* (*)(UClass* cls,
                                        UObject* obj,
                                        FName name,
                                        uint32_t flags,
                                        uint32_t internal_flags,
                                        UObject* template_obj,
                                        uint32_t copy_transients_from_class_defaults,
                                        void* instance_graph,
                                        uint32_t assume_template_is_archetype);
construct_obj_func construct_obj_ptr;

}  // namespace

void BL3Hook::find_construct_object(void) {
    static const Pattern CONSTRUCT_OBJECT_PATTERN{
        "\xE8\x00\x00\x00\x00\x41\x89\x3E\x4D\x8D\x46\x04",
        "\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 1};

    auto construct_obj_instr = sigscan(CONSTRUCT_OBJECT_PATTERN);
    construct_obj_ptr = read_offset<construct_obj_func>(construct_obj_instr);
    LOG(MISC, "StaticConstructObject: {:p}", reinterpret_cast<void*>(construct_obj_ptr));
}

UObject* BL3Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   decltype(UObject::ObjectFlags) flags,
                                   UObject* template_obj) const {
    return construct_obj_ptr(cls, outer, name, flags, 0, template_obj, 0 /* false */, nullptr,
                             0 /* false */);
}

#pragma endregion

#pragma region PathName

namespace {

using get_path_name_func = void (*)(const UObject* self,
                                    const UObject* stop_outer,
                                    ManagedFString* str);
get_path_name_func get_path_name_ptr;

}  // namespace

void BL3Hook::find_get_path_name(void) {
    static const Pattern GET_PATH_NAME_PATTERN{
        "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x57\x48\x83\xEC\x20\x49\x8B\xF8\x48\x8B\xE9",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    get_path_name_ptr = sigscan<get_path_name_func>(GET_PATH_NAME_PATTERN);
    LOG(MISC, "GetPathName: {:p}", reinterpret_cast<void*>(get_path_name_ptr));
}

std::wstring BL3Hook::uobject_path_name(const UObject* obj) const {
    ManagedFString str{};
    get_path_name_ptr(obj, nullptr, &str);
    return str;
}

#pragma endregion

#pragma region StaticFindObject

namespace {

using static_find_object_safe_func = UObject* (*)(const UClass* cls,
                                                  intptr_t package,
                                                  const wchar_t* str,
                                                  uint32_t exact_class);
static_find_object_safe_func static_find_object_ptr;

}  // namespace

void BL3Hook::find_static_find_object(void) {
    static const Pattern STATIC_FIND_OBJECT_PATTERN{
        "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x30\x80\x3D"
        "\x00\x00\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\x00\x00\x00\x00\xFF"};

    static_find_object_ptr = sigscan<static_find_object_safe_func>(STATIC_FIND_OBJECT_PATTERN);
    LOG(MISC, "StaticFindObjectSafe: {:p}", reinterpret_cast<void*>(static_find_object_ptr));
}

UObject* BL3Hook::find_object(UClass* cls, const std::wstring& name) const {
    static constexpr intptr_t ANY_PACKAGE = -1;
    return static_find_object_ptr(cls, ANY_PACKAGE, name.c_str(), 0 /* false */);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
