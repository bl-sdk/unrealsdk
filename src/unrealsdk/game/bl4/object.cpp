#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl4/bl4.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/multi_sigscan.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/ffield.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unrealsdk.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2 && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

#pragma region PathName

namespace {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

// TODO: May want to move this type into a generic header at some point
// NOLINTNEXTLINE(readability-identifier-naming)
struct TStringBuilderBase_wchar_t {
    wchar_t* base;
    wchar_t* current;
    wchar_t* end;
    uint32_t is_dynamic;
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

using get_obj_path_name_func = void (*)(const UObject* self,
                                        const UObject* stop_outer,
                                        TStringBuilderBase_wchar_t* str);
get_obj_path_name_func get_obj_path_name_ptr;

// Search for the string L"Cannot replace existing object of a different class.", xrefs
// This error message contains two object path names, so it calls get path twice before
// The get path you get from there is just the raw fstring version (?), and first function called
// by it is the inner string builder we hook here
const constexpr Pattern<39> GET_OBJ_PATH_NAME_PGO_PATTERN{
    "41 56"                 // push r14
    "56"                    // push rsi
    "57"                    // push rdi
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000088
    "4C 89 C6"              // mov rsi, r8
    "48 89 CF"              // mov rdi, rcx
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+11399940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000080], rax
    "48 39 D1"              // cmp rcx, rdx
};
const constexpr Pattern<42> GET_OBJ_PATH_NAME_NON_PGO_PATTERN{
    "41 56"                 // push r14
    "56"                    // push rsi
    "57"                    // push rdi
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000088
    "4C 89 C6"              // mov rsi, r8
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+C372940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000080], rax
    "48 85 C9"              // test rcx, rcx
    "0F84 ????????"         // je Borderlands4.exe+4261FF6
};

using get_field_path_name_func = ManagedFString* (*)(const FField* self,
                                                     ManagedFString* ret,
                                                     UObject* stop_outer);
get_field_path_name_func get_field_path_name_ptr;

// Search for the string "CoreUObject/Private/UObject/PropertyBaseObject.cpp", xrefs
// It calls UObjectBaseUtility::GetPathName, then FField::GetPathName, then the log function
const constexpr Pattern<43> GET_FIELD_PATH_NAME_PGO_PATTERN{
    "41 57"                 // push r15
    "41 56"                 // push r14
    "56"                    // push rsi
    "57"                    // push rdi
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000250
    "4C 89 C0"              // mov rax, r8
    "48 89 D6"              // mov rsi, rdx
    "48 8B 15 ????????"     // mov rdx, [Borderlands4.exe+11399940]
    "48 31 E2"              // xor rdx, rsp
    "48 89 94 24 ????????"  // mov [rsp+00000248], rdx
    "48 8D 54 24 ??"        // lea rdx,[rsp+58]
};
const constexpr Pattern<67> GET_FIELD_PATH_NAME_NON_PGO_PATTERN{
    "56"                    // push rsi
    "57"                    // push rdi
    "48 81 EC ????????"     // sub rsp, 00000258
    "48 89 D6"              // mov rsi, rdx
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+C372940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000250], rax
    "48 8D 44 24 ??"        // lea rax, [rsp+50]
    "C6 40 F8 00"           // mov byte ptr [rax-08], 00
    "48 89 40 E0"           // mov [rax-20], rax
    "48 89 40 E8"           // mov [rax-18], rax
    "48 8D 94 24 ????????"  // lea rdx, [rsp+00000250]
    "48 89 50 F0"           // mov [rax-10], rdx
    "48 8D 7C 24 ??"        // lea rdi, [rsp+30]
    "4C 89 C2"              // mov rdx, r8
};

}  // namespace
namespace bl4 {
constinit MultiPattern get_obj_path_name_pgo_multi{GET_OBJ_PATH_NAME_PGO_PATTERN};
constinit MultiPattern get_obj_path_name_non_pgo_multi{GET_OBJ_PATH_NAME_NON_PGO_PATTERN};
constinit MultiPattern get_field_path_name_pgo_multi{GET_FIELD_PATH_NAME_PGO_PATTERN};
constinit MultiPattern get_field_path_name_non_pgo_multi{GET_FIELD_PATH_NAME_NON_PGO_PATTERN};
}  // namespace bl4

void BL4Hook::find_get_path_name(void) {
    get_obj_path_name_ptr = BL4Hook::choose_pattern<get_obj_path_name_func>(
        bl4::get_obj_path_name_pgo_multi, bl4::get_obj_path_name_non_pgo_multi,
        "UObject::GetPathName");

    get_field_path_name_ptr = BL4Hook::choose_pattern<get_field_path_name_func>(
        bl4::get_field_path_name_pgo_multi, bl4::get_field_path_name_non_pgo_multi,
        "FField::GetPathName");
}

std::wstring BL4Hook::uobject_path_name(const UObject* obj) const {
    const constexpr auto default_buf_size = 256;

    wchar_t buf[default_buf_size];
    TStringBuilderBase_wchar_t str{
        .base = &buf[0],
        .current = &buf[0],
        // Assuming this is an exclusive end, there's code like `if (end <= current + 4)`
        .end = &buf[default_buf_size],
        .is_dynamic = 0,
    };

    get_obj_path_name_ptr(obj, nullptr, &str);

    std::wstring output{str.base, static_cast<size_t>(str.current - str.base)};
    if (str.is_dynamic != 0) {
        unrealsdk::u_free(str.base);
    }

    return output;
}

std::wstring BL4Hook::ffield_path_name(const FField* field) const {
    ManagedFString str;
    get_field_path_name_ptr(field, &str, nullptr);
    return str;
}

#pragma endregion

#pragma region ConstructObject

namespace {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
// NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)

struct FStaticConstructObjectParameters {
    UClass* Class{};
    UObject* Outer{};
    FName Name;
    uint32_t SetFlags{};
    int InternalSetFlags{};
    bool bCopyTransientsFromClassDefaults{};
    bool bAssumeTemplateIsArchetype{};
    UObject* Template{};
    void* InstanceGraph{};
    void* ExternalPackage{};
    void* PropertyInitCallback{};
    void* SubobjectOverrides{};

    // Seems to need some extra space, not sure exactly how big this should be.
    char dummy[0x40]{};
};

// NOLINTEND(readability-identifier-naming, readability-magic-numbers)
UNREALSDK_UNREAL_STRUCT_PADDING_POP()

using construct_obj_func = UObject* (*)(FStaticConstructObjectParameters * params);
construct_obj_func construct_obj_ptr;

const constexpr Pattern<41> CONSTRUCT_OBJECT_PGO_PATTERN{
    "41 56"                 // push r14
    "56"                    // push rsi
    "57"                    // push rdi
    "55"                    // push rbp
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000280
    "48 89 CE"              // mov rsi, rcx
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+11399940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000278], rax
    "48 8B 39"              // mov rdi, [rcx]
    "48 8B 51 08"           // mov rdx, [rcx+08]
};
const constexpr Pattern<39> CONSTRUCT_OBJECT_NON_PGO_PATTERN{
    "41 57"                 // push r15
    "41 56"                 // push r14
    "56"                    // push rsi
    "57"                    // push rdi
    "55"                    // push rbp
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000278
    "48 89 CE"              // mov rsi, rcx
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+C372940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000270], rax
    "48 8B 39"              // mov rdi, [rcx]
};

}  // namespace
namespace bl4 {
constinit MultiPattern construct_obj_pgo_multi{CONSTRUCT_OBJECT_PGO_PATTERN};
constinit MultiPattern construct_obj_non_pgo_multi{CONSTRUCT_OBJECT_NON_PGO_PATTERN};
}  // namespace bl4

void BL4Hook::find_construct_object(void) {
    construct_obj_ptr = BL4Hook::choose_pattern<construct_obj_func>(
        bl4::construct_obj_pgo_multi, bl4::construct_obj_non_pgo_multi, "StaticConstructObject");
}

UObject* BL4Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   uint64_t flags,
                                   UObject* template_obj) const {
    if (flags > std::numeric_limits<uint32_t>::max()) {
        throw std::out_of_range("construct_object flags out of range, only 32-bits are supported");
    }

    FStaticConstructObjectParameters params{};
    params.Class = cls;
    params.Outer = outer;
    params.Name = name;
    params.SetFlags = static_cast<uint32_t>(flags);
    params.InternalSetFlags = 0;
    params.Template = template_obj;

    return construct_obj_ptr(&params);
}

#pragma endregion

#pragma region StaticFindObject

namespace {

using static_find_object_safe_func = UObject* (*)(const UClass* cls,
                                                  intptr_t package,
                                                  const wchar_t* str,
                                                  uint32_t exact_class);
static_find_object_safe_func static_find_object_ptr;

// Search for the string L"FindImportedObject", xrefs. This function makes several calls to it,
// easiest to find is near the top of the function, first call inside the loop.
const constexpr Pattern<35> STATIC_FIND_OBJECT_PGO_PATTERN{
    "41 56"              // push r14
    "56"                 // push rsi
    "57"                 // push rdi
    "53"                 // push rbx
    "48 83 EC ??"        // sub rsp, 38
    "48 8B 05 ????????"  // mov rax, [Borderlands4.exe+11399940]
    "48 31 E0"           // xor rax, rsp
    "48 89 44 24 ??"     // mov [rsp+30], rax
    "F6 05 ???????? 01"  // test byte ptr [Borderlands4.exe+115D7A00], 01
    "74 ??"              // jz short loc_14153EB78
    "31 C0"              // xor eax, eax
};
const constexpr Pattern<32> STATIC_FIND_OBJECT_NON_PGO_PATTERN{
    "41 57"              // push r15
    "41 56"              // push r14
    "41 55"              // push r13
    "41 54"              // push r12
    "56"                 // push rsi
    "57"                 // push rdi
    "55"                 // push rbp
    "53"                 // push rbx
    "48 83 EC 28"        // sub rsp, 28
    "F6 05 ???????? 01"  // test byte ptr [Borderlands4.exe+C5B2940], 01
    "0F 85 ????????"     // jnz loc_14005F0E1
    "44 89 CB"           // mov ebx, r9d
};

const constexpr intptr_t ANY_PACKAGE = -1;

}  // namespace
namespace bl4 {
constinit MultiPattern find_obj_pgo_multi{STATIC_FIND_OBJECT_PGO_PATTERN};
constinit MultiPattern find_obj_non_pgo_multi{STATIC_FIND_OBJECT_NON_PGO_PATTERN};
}  // namespace bl4

void BL4Hook::find_static_find_object(void) {
    static_find_object_ptr = BL4Hook::choose_pattern<static_find_object_safe_func>(
        bl4::find_obj_pgo_multi, bl4::find_obj_non_pgo_multi, "StaticFindObjectSafe");
}

UObject* BL4Hook::find_object(UClass* cls, const std::wstring& name) const {
    return static_find_object_ptr(cls, ANY_PACKAGE, name.c_str(), 0 /* false */);
}

#pragma endregion

#pragma region LoadPackage

namespace {

using load_package_func = UObject* (*)(const UObject* outer,
                                       const wchar_t* name,
                                       uint32_t flags,
                                       void* instancing_context,
                                       void* reader_override,
                                       void* diff_package_path);
load_package_func load_package_ptr;

const constexpr Pattern<50> LOAD_PACKAGE_PATTERN{
    "41 57"                 // push r15
    "41 56"                 // push r14
    "41 55"                 // push r13
    "41 54"                 // push r12
    "56"                    // push rsi
    "57"                    // push rdi
    "55"                    // push rbp
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000188
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+C372940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000180], rax
    "48 83 3A 00"           // cmp qword ptr [rdx], 00
    "0F84 ????????"         // je Borderlands4.exe+2546480
    "4D 89 CF"              // mov r15, r9
};

}  // namespace
namespace bl4 {
constinit MultiPattern load_package_multi{LOAD_PACKAGE_PATTERN};
}

void BL4Hook::find_load_package(void) {
    load_package_ptr = LOAD_PACKAGE_PATTERN.sigscan_nullable<load_package_func>();
    LOG(MISC, "LoadPackage: {:p}", reinterpret_cast<void*>(load_package_ptr));
}

[[nodiscard]] UObject* BL4Hook::load_package(const std::wstring& name, uint32_t flags) const {
    return load_package_ptr(nullptr, name.data(), flags, nullptr, nullptr, nullptr);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
