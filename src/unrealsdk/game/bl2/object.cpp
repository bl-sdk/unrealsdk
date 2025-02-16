#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

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

const constinit Pattern<49> CONSTRUCT_OBJECT_PATTERN{
    "55"              // push ebp
    "8B EC"           // mov ebp, esp
    "6A FF"           // push -01
    "68 ????????"     // push Borderlands2.exe+1107DCB
    "64 A1 ????????"  // mov eax, fs:[00000000]
    "50"              // push eax
    "83 EC 10"        // sub esp, 10
    "53"              // push ebx
    "56"              // push esi
    "57"              // push edi
    "A1 ????????"     // mov eax, [Borderlands2.g_LEngineDefaultPoolId+B2DC]
    "33 C5"           // xor eax, ebp
    "50"              // push eax
    "8D 45 ??"        // lea eax, [ebp-0C]
    "64 A3 ????????"  // mov fs:[00000000], eax
    "8B 7D ??"        // mov edi, [ebp+08]
    "8A 87 ????????"  // mov al, [edi+000001CC]
};

}  // namespace

void BL2Hook::find_construct_object(void) {
    construct_obj_ptr = CONSTRUCT_OBJECT_PATTERN.sigscan_nullable<construct_obj_func>();
    LOG(MISC, "StaticConstructObject: {:p}", reinterpret_cast<void*>(construct_obj_ptr));
}

UObject* BL2Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   uint64_t flags,
                                   UObject* template_obj) const {
    return construct_obj_ptr(cls, outer, name, flags, template_obj, nullptr, nullptr,
                             0 /* false */);
}

#pragma endregion

#pragma region PathName

// UObject::PathName is exposed to UnrealScript, we *can* get away with just calling it.
// That has a decent bit of overhead however, especially if process event is locking, and this
// function is used to tell if to call a hook, it's called all the time.
// Stick with a native function call for speed.

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

const constinit Pattern<15> GET_PATH_NAME_PATTERN{
    "55"        // push ebp
    "8B EC"     // mov ebp, esp
    "8B 45 ??"  // mov eax, [ebp+08]
    "56"        // push esi
    "8B F1"     // mov esi, ecx
    "3B F0"     // cmp esi, eax
    "74 ??"     // je Borderlands2.exe+ADB04
    "85 F6"     // test esi, esi
};

}  // namespace

void BL2Hook::find_get_path_name(void) {
    get_path_name_ptr = GET_PATH_NAME_PATTERN.sigscan_nullable<get_path_name_func>();
    LOG(MISC, "GetPathName: {:p}", reinterpret_cast<void*>(get_path_name_ptr));
}

std::wstring BL2Hook::uobject_path_name(const UObject* obj) const {
    ManagedFString str{};
    get_path_name_ptr(obj, nullptr, &str);
    return str;
}

#pragma endregion

#pragma region FindObject

// Again UObject::FindObject is exposed to UnrealScript. We don't care as much about performance
// here, but may as well still use a native call, was easy enough to find.

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject*(__cdecl* static_find_object_func)(const UClass* cls,
                                                   const UObject* package,
                                                   const wchar_t* str,
                                                   uint32_t exact_class);

static_find_object_func static_find_object_ptr;
const constinit Pattern<56> STATIC_FIND_OBJECT_PATTERN{
    "55"                 // push ebp
    "8B EC"              // mov ebp, esp
    "6A FF"              // push -01
    "68 ????????"        // push Borderlands2.exe+1106400
    "64 A1 ????????"     // mov eax, fs:[00000000]
    "50"                 // push eax
    "83 EC 24"           // sub esp, 24
    "53"                 // push ebx
    "56"                 // push esi
    "57"                 // push edi
    "A1 ????????"        // mov eax, [Borderlands2.g_LEngineDefaultPoolId+B2DC]
    "33 C5"              // xor eax, ebp
    "50"                 // push eax
    "8D 45 ??"           // lea eax, [ebp-0C]
    "64 A3 ????????"     // mov fs:[00000000], eax
    "83 3D ???????? 00"  // cmp dword ptr [Borderlands2.exe+1682B14], 00
    "75 ??"              // jne Borderlands2.GetOutermost+429A
    "83 3D ???????? 00"  // cmp dword ptr [Borderlands2.exe+15E801C], 00
};

}  // namespace

void BL2Hook::find_static_find_object(void) {
    static_find_object_ptr = STATIC_FIND_OBJECT_PATTERN.sigscan_nullable<static_find_object_func>();
    LOG(MISC, "StaticFindObject: {:p}", reinterpret_cast<void*>(static_find_object_ptr));
}

UObject* BL2Hook::find_object(UClass* cls, const std::wstring& name) const {
    return static_find_object_ptr(cls, nullptr, name.c_str(), 0 /* false */);
}

#pragma endregion

#pragma region LoadPackage

namespace {

using load_package_func = UObject* (*)(const UObject* outer, const wchar_t* name, uint32_t flags);
load_package_func load_package_ptr;

const constinit Pattern<46> LOAD_PACKAGE_PATTERN{
    "55"              // push ebp
    "8B EC"           // mov ebp, esp
    "6A FF"           // push -01
    "68 ????????"     // push Borderlands2.exe+1108180
    "64 A1 ????????"  // mov eax, fs:[00000000]
    "50"              // push eax
    "83 EC 68"        // sub esp, 68
    "A1 ????????"     // mov eax, [Borderlands2.g_LEngineDefaultPoolId+B2DC]
    "33 C5"           // xor eax, ebp
    "89 45 ??"        // mov [ebp-14], eax
    "53"              // push ebx
    "56"              // push esi
    "57"              // push edi
    "50"              // push eax
    "8D 45 ??"        // lea eax, [ebp-0C]
    "64 A3 ????????"  // mov fs:[00000000], eax
    "89 65 ??"        // mov [ebp-10], esp
};

}  // namespace

void BL2Hook::find_load_package(void) {
    load_package_ptr = LOAD_PACKAGE_PATTERN.sigscan_nullable<load_package_func>();
    LOG(MISC, "LoadPackage: {:p}", reinterpret_cast<void*>(load_package_ptr));
}

[[nodiscard]] UObject* BL2Hook::load_package(const std::wstring& name, uint32_t flags) const {
    return load_package_ptr(nullptr, name.data(), flags);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
