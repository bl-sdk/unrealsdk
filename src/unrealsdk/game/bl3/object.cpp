#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

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
    construct_obj_ptr = CONSTRUCT_OBJECT_PATTERN.sigscan_nullable<construct_obj_func>();
    LOG(MISC, "StaticConstructObject: {:p}", reinterpret_cast<void*>(construct_obj_ptr));
}

UObject* BL3Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   uint64_t flags,
                                   UObject* template_obj) const {
    if (flags > std::numeric_limits<uint32_t>::max()) {
        throw std::out_of_range("construct_object flags out of range, only 32-bits are supported");
    }
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
    get_path_name_ptr = GET_PATH_NAME_PATTERN.sigscan_nullable<get_path_name_func>();
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
    static_find_object_ptr =
        STATIC_FIND_OBJECT_PATTERN.sigscan_nullable<static_find_object_safe_func>();
    LOG(MISC, "StaticFindObjectSafe: {:p}", reinterpret_cast<void*>(static_find_object_ptr));
}

UObject* BL3Hook::find_object(UClass* cls, const std::wstring& name) const {
    return static_find_object_ptr(cls, ANY_PACKAGE, name.c_str(), 0 /* false */);
}

#pragma endregion

#pragma region LoadPackage

namespace {

using load_package_func = UObject* (*)(const UObject* outer,
                                       const wchar_t* name,
                                       uint32_t flags,
                                       void* reader_override);
load_package_func load_package_ptr;

const constinit Pattern<16> LOAD_PACKAGE_PATTERN{
    "48 8B C4"     // mov rax, rsp
    "53"           // push rbx
    "56"           // push rsi
    "48 83 EC 68"  // sub rsp, 68
    "48 89 68 ??"  // mov [rax+08], rbp
    "48 8B EA"     // mov rbp, rdx
};

}  // namespace

void BL3Hook::find_load_package(void) {
    load_package_ptr = LOAD_PACKAGE_PATTERN.sigscan_nullable<load_package_func>();
    LOG(MISC, "LoadPackage: {:p}", reinterpret_cast<void*>(load_package_ptr));
}

[[nodiscard]] UObject* BL3Hook::load_package(const std::wstring& name, uint32_t flags) const {
    return load_package_ptr(nullptr, name.data(), flags, nullptr);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
