#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unrealsdk.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

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
    find_ftext_as_culture_invariant();

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

const constinit Pattern<15> FNAME_INIT_PATTERN{
    "48 89 5C 24 ??"     // mov [rsp+18], rbx
    "55"                 // push rbp
    "56"                 // push rsi
    "57"                 // push rdi
    "48 81 EC 60080000"  // sub rsp, 00000860
};

}  // namespace

void BL3Hook::find_fname_init(void) {
    fname_init_ptr = FNAME_INIT_PATTERN.sigscan<fname_init_func>();
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

const constinit Pattern<10> FFRAME_STEP_SIG{
    "48 8B 41 ??"  // mov rax, [rcx+20]
    "4C 8B D2"     // mov r10, rdx
    "48 8B D1"     // mov rdx, rcx
};

}  // namespace

void BL3Hook::find_fframe_step(void) {
    fframe_step_ptr = FFRAME_STEP_SIG.sigscan<fframe_step_func>();
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

const constinit Pattern<55> CONSTRUCT_OBJECT_PATTERN{
    "48 89 5C 24 18"        // mov [rsp+18], rbx
    "55"                    // push rbp
    "56"                    // push rsi
    "57"                    // push rdi
    "41 54"                 // push r12
    "41 55"                 // push r13
    "41 56"                 // push r14
    "41 57"                 // push r15
    "48 8D AC 24 ????????"  // lea rbp, [rsp-000000C0]
    "48 81 EC C0010000"     // sub rsp, 000001C0
    "48 8B 05 ????????"     // mov rax, [Borderlands3.exe+683B348]
    "48 33 C4"              // xor rax, rsp
    "48 89 85 ????????"     // mov [rbp+000000B0], rax
    "44 8B A5 ????????"     // mov r12d, [rbp+00000120]
};

}  // namespace

void BL3Hook::find_construct_object(void) {
    construct_obj_ptr = CONSTRUCT_OBJECT_PATTERN.sigscan<construct_obj_func>();
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

const constinit Pattern<21> GET_PATH_NAME_PATTERN{
    "48 89 5C 24 ??"  // mov [rsp+18], rbx
    "48 89 6C 24 ??"  // mov [rsp+20], rbp
    "57"              // push rdi
    "48 83 EC 20"     // sub rsp, 20
    "49 8B F8"        // mov rdi, r8
    "48 8B E9"        // mov rbp, rcx
};

}  // namespace

void BL3Hook::find_get_path_name(void) {
    get_path_name_ptr = GET_PATH_NAME_PATTERN.sigscan<get_path_name_func>();
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

const constinit Pattern<27> STATIC_FIND_OBJECT_PATTERN{
    "48 89 5C 24 ??"     // mov [rsp+08], rbx
    "48 89 6C 24 ??"     // mov [rsp+10], rbp
    "48 89 74 24 ??"     // mov [rsp+18], rsi
    "57"                 // push rdi
    "48 83 EC 30"        // sub rsp, 30
    "80 3D ???????? 00"  // cmp byte ptr [Borderlands3.exe+69EAA10], 00
};

const constexpr intptr_t ANY_PACKAGE = -1;

}  // namespace

void BL3Hook::find_static_find_object(void) {
    static_find_object_ptr = STATIC_FIND_OBJECT_PATTERN.sigscan<static_find_object_safe_func>();
    LOG(MISC, "StaticFindObjectSafe: {:p}", reinterpret_cast<void*>(static_find_object_ptr));
}

UObject* BL3Hook::find_object(UClass* cls, const std::wstring& name) const {
    return static_find_object_ptr(cls, ANY_PACKAGE, name.c_str(), 0 /* false */);
}

#pragma endregion

#pragma region FText::AsCultureInvariant

namespace {

using ftext_as_culture_invariant_func = void (*)(FText* self, const TemporaryFString* str);
ftext_as_culture_invariant_func ftext_as_culture_invariant_ptr;

const constinit Pattern<131> FTEXT_AS_CULTURE_INVARIANT_PATTERN{
    "48 89 5C 24 ??"  // mov [rsp+08], rbx
    "48 89 74 24 ??"  // mov [rsp+10], rsi
    "57"              // push rdi
    "48 83 EC 30"     // sub rsp, 30
    "48 63 7A ??"     // movsxd rdi, dword ptr [rdx+08]
    "33 C0"           // xor eax, eax
    "48 8B 32"        // mov rsi, [rdx]
    "48 8B D9"        // mov rbx, rcx
    "48 89 44 24 ??"  // mov [rsp+20], rax
    "89 7C 24 ??"     // mov [rsp+28], edi
    "85 FF"           // test edi, edi
    "75 ??"           // jne Borderlands3.exe+15F7D6E
    "89 44 24 ??"     // mov [rsp+2C], eax
    "EB ??"           // jmp Borderlands3.exe+15F7D90
    "45 33 C0"        // xor r8d, r8d
    "48 8D 4C 24 ??"  // lea rcx, [rsp+20]
    "8B D7"           // mov edx, edi
    "E8 ????????"     // call Borderlands3.AK::MemoryMgr::StartProfileThreadUsage+CFD0
    "48 8B 4C 24 ??"  // mov rcx, [rsp+20]
    "4C 8B C7"        // mov r8, rdi
    "4D 03 C0"        // add r8, r8
    "48 8B D6"        // mov rdx, rsi
    "E8 ????????"     // call Borderlands3.exe+3DDBB70 { ->->VCRUNTIME140.memcpy }
    "48 8D 54 24 ??"  // lea rdx, [rsp+20]
    "48 8B CB"        // mov rcx, rbx
    "E8 ????????"     // call Borderlands3.exe+15EC100
    "48 8B 4C 24 ??"  // mov rcx, [rsp+20]
    "48 85 C9"        // test rcx, rcx
    "74 05"           // je Borderlands3.exe+15F7DAC
    "E8 ????????"     // call Borderlands3.exe+15D37B0
    "83 4B ?? 02"     // or dword ptr [rbx+10], 02
    // Note that the actual required signature ends here
    // Since we're so close to the end, might as well just finish the function though
    "48 8B C3"        // mov rax, rbx
    "48 8B 5C 24 ??"  // mov rbx, [rsp+40]
    "48 8B 74 24 ??"  // mov rsi, [rsp+48]
    "48 83 C4 30"     // add rsp, 30
    "5F"              // pop rdi
    "C3"              // ret
};

}  // namespace

void BL3Hook::find_ftext_as_culture_invariant(void) {
    ftext_as_culture_invariant_ptr =
        FTEXT_AS_CULTURE_INVARIANT_PATTERN.sigscan<ftext_as_culture_invariant_func>();
    LOG(MISC, "FText::AsCultureInvariant: {:p}", reinterpret_cast<void*>(fname_init_ptr));
}

void BL3Hook::ftext_as_culture_invariant(FText* text, TemporaryFString&& str) const {
    ftext_as_culture_invariant_ptr(text, &str);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
