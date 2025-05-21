#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"
#include "unrealsdk/logging.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/structs/fstring.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

#pragma region ConstructObject

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject*(__cdecl* construct_obj_func)(UClass* cls,
                                              UObject* outer,
                                              FName name,
                                              uint64_t flags,
                                              UObject* template_obj,
                                              void* error_output_device,
                                              void* instance_graph,
                                              uint32_t assume_template_is_archetype);
construct_obj_func construct_obj_ptr;

const constinit Pattern<47> CONSTRUCT_OBJECT_PATTERN{
    "6A FF"           // push -01
    "68 ????????"     // push 018E9573
    "64 A1 ????????"  // mov eax, fs:[00000000]
    "50"              // push eax
    "83 EC 0C"        // sub esp, 0C
    "53"              // push ebx
    "55"              // push ebp
    "56"              // push esi
    "57"              // push edi
    "A1 ????????"     // mov eax, [01F131C0]
    "33 C4"           // xor eax, esp
    "50"              // push eax
    "8D 44 24 ??"     // lea eax, [esp+20]
    "64 A3 ????????"  // mov fs:[00000000], eax
    "8B 6C 24 ??"     // mov ebp, [esp+54]
    "89 6C 24 ??"     // mov [esp+14], ebp
};

}  // namespace

void BL1Hook::find_construct_object(void) {
    construct_obj_ptr = CONSTRUCT_OBJECT_PATTERN.sigscan_nullable<construct_obj_func>();
    LOG(MISC, "StaticConstructObject: {:p}", reinterpret_cast<void*>(construct_obj_ptr));
}

UObject* BL1Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   uint64_t flags,
                                   UObject* template_obj) const {
    return construct_obj_ptr(cls, outer, name, flags, template_obj, nullptr, nullptr,
                             0 /* false */);
}

#pragma endregion

#pragma region PathName

namespace {

#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"  // thiscall on non-class
#endif

// NOLINTNEXTLINE(modernize-use-using)
typedef void(__thiscall* get_path_name_func)(const UObject* self,
                                             const UObject* stop_outer,
                                             ManagedFString* str);

#if defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

get_path_name_func get_path_name_ptr;

const constinit Pattern<13> GET_PATH_NAME_PATTERN{
    "8B 44 24 ??"  // mov eax, [esp+04]
    "56"           // push esi
    "8B F1"        // mov esi, ecx
    "3B F0"        // cmp esi, eax
    "74 ??"        // je 005D09D1
    "85 F6"        // test esi, esi
};

}  // namespace

void BL1Hook::find_get_path_name(void) {
    get_path_name_ptr = GET_PATH_NAME_PATTERN.sigscan_nullable<get_path_name_func>();
    LOG(MISC, "GetPathName: {:p}", reinterpret_cast<void*>(get_path_name_ptr));
}

std::wstring BL1Hook::uobject_path_name(const UObject* obj) const {
    ManagedFString str{};
    get_path_name_ptr(obj, nullptr, &str);
    return str;
}

#pragma endregion

#pragma region FindObject

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject*(__cdecl* static_find_object_func)(const UClass* cls,
                                                   const UObject* package,
                                                   const wchar_t* str,
                                                   uint32_t exact_class);

static_find_object_func static_find_object_ptr;
const constinit Pattern<47> STATIC_FIND_OBJECT_PATTERN{
    "6A FF"           // push -01
    "68 ????????"     // push 018E7FF0
    "64 A1 ????????"  // mov eax, fs:[00000000]
    "50"              // push eax
    "83 EC 24"        // sub esp, 24
    "53"              // push ebx
    "55"              // push ebp
    "56"              // push esi
    "57"              // push edi
    "A1 ????????"     // mov eax, [01F131C0]
    "33 C4"           // xor eax, esp
    "50"              // push eax
    "8D 44 24 ??"     // lea eax, [esp+38]
    "64 A3 ????????"  // mov fs:[00000000], eax
    "8B 74 24 ??"     // mov esi, [esp+4C]
    "8B 7C 24 ??"     // mov edi, [esp+50]
};

}  // namespace

void BL1Hook::find_static_find_object(void) {
    static_find_object_ptr = STATIC_FIND_OBJECT_PATTERN.sigscan_nullable<static_find_object_func>();
    LOG(MISC, "StaticFindObject: {:p}", reinterpret_cast<void*>(static_find_object_ptr));
}

UObject* BL1Hook::find_object(UClass* cls, const std::wstring& name) const {
    return static_find_object_ptr(cls, nullptr, name.c_str(), 0 /* false */);
}

#pragma endregion

#pragma region LoadPackage

namespace {

using load_package_func = UObject* (*)(const UObject* outer, const wchar_t* name, uint32_t flags);
load_package_func load_package_ptr;

const constinit Pattern<21> LOAD_PACKAGE_PATTERN{
    "55"              // push ebp
    "8B EC"           // mov ebp, esp
    "6A FF"           // push -01
    "68 ????????"     // push 018E8C90
    "64 A1 ????????"  // mov eax, fs:[00000000]
    "50"              // push eax
    "83 EC 28"        // sub esp, 28
    "53"              // push ebx
};

}  // namespace

void BL1Hook::find_load_package(void) {
    load_package_ptr = LOAD_PACKAGE_PATTERN.sigscan_nullable<load_package_func>();
    LOG(MISC, "LoadPackage: {:p}", reinterpret_cast<void*>(load_package_ptr));
}

[[nodiscard]] UObject* BL1Hook::load_package(const std::wstring& name, uint32_t flags) const {
    return load_package_ptr(nullptr, name.data(), flags);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
