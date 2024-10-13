#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"
#include "unrealsdk/logging.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/structs/fstring.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING) \
    && defined(UNREALSDK_GAME_BL1)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

// ############################################################################//
//  | CONSTRUCT OBJECT |
// ############################################################################//

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
    "6A FF"           // push FFFFFFFF
    "68 ????????"     // push borderlands.18ED693
    "64A1 00000000"   // mov eax,dword ptr fs:[0]
    "50"              // push eax
    "83EC 0C"         // sub esp,C
    "53"              // push ebx
    "55"              // push ebp
    "56"              // push esi
    "57"              // push edi
    "A1 ????????"     // mov eax,dword ptr ds:[1F16980]
    "33C4"            // xor eax,esp
    "50"              // push eax
    "8D4424 20"       // lea eax,dword ptr ss:[esp+20]
    "64 A3 00000000"  // mov dword ptr fs:[0],eax
    "8B6C24 54"       // mov ebp,dword ptr ss:[esp+54]
    "896C24 14"       // mov dword ptr ss:[esp+14],ebp
    "85ED"            // test ebp,ebp
};

}  // namespace

void BL1Hook::find_construct_object(void) {
    construct_obj_ptr = CONSTRUCT_OBJECT_PATTERN.sigscan_nullable<construct_obj_func>();
    LOG(MISC, "StaticConstructObject: {:p}", reinterpret_cast<void*>(construct_obj_ptr));
}

UObject* BL1Hook::construct_object(UClass* cls,
                                   UObject* outer,
                                   const FName& name,
                                   decltype(UObject::ObjectFlags) flags,
                                   UObject* template_obj) const {
    return construct_obj_ptr(cls, outer, name, flags, template_obj, nullptr, nullptr,
                             0 /* false */);
}

#pragma endregion

// ############################################################################//
//  | PATH NAME |
// ############################################################################//

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

const constinit Pattern<19> GET_PATH_NAME_PATTERN{
    "8B4424 04"  // mov eax,dword ptr ss:[esp+4]
    "56"         // push esi
    "8BF1"       // mov esi,ecx
    "3BF0"       // cmp esi,eax
    "74 76"      // je borderlands.5CF501
    "85F6"       // test esi,esi
    "74 72"      // je borderlands.5CF501
    "8B4E 28"    // mov ecx,dword ptr ds:[esi+28]
    "53"         // push ebx
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

// ############################################################################//
//  | FIND OBJECT |
// ############################################################################//

#pragma region FindObject

namespace {

// NOLINTNEXTLINE(modernize-use-using)
typedef UObject*(__cdecl* static_find_object_func)(const UClass* cls,
                                                   const UObject* package,
                                                   const wchar_t* str,
                                                   uint32_t exact_class);

static_find_object_func static_find_object_ptr;
const constinit Pattern<50> STATIC_FIND_OBJECT_PATTERN{
    "6A FF"          // push FFFFFFFF
    "68 ????????"    // push borderlands.18EC190
    "64A1 00000000"  // mov eax,dword ptr fs:[0]
    "50"             // push eax
    "83EC 24"        // sub esp,24
    "53"             // push ebx
    "55"             // push ebp
    "56"             // push esi
    "57"             // push edi
    "A1 ????????"    // mov eax,dword ptr ds:[1F16980]
    "33 C4"          // xor eax,esp
    "50"             // push eax
    "8D4424 38"      // lea eax,dword ptr ss:[esp+38]
    "64A3 00000000"  // mov dword ptr fs:[0],eax
    "8B7424 4C"      // mov esi,dword ptr ss:[esp+4C]
    "8B7C24 50"      // mov edi,dword ptr ss:[esp+50]
    "8BC6"           // mov eax,esi
    "40"             // inc eax
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

// ############################################################################//
//  | LOAD PACKAGE |
// ############################################################################//

#pragma region LoadPackage

namespace {

using load_package_func = UObject* (*)(const UObject* outer, const wchar_t* name, uint32_t flags);
load_package_func load_package_ptr;

const constinit Pattern<34> LOAD_PACKAGE_PATTERN{
    "55"              // push ebp
    "8B EC"           // mov ebp,esp
    "6A FF"           // push FFFFFFFF
    "68 ????????"     // push borderlands.18ECDB0
    "64 A1 00000000"  // mov eax,dword ptr fs:[0]
    "50"              // push eax
    "83EC 28"         // sub esp,28
    "53 56 57"        // push ebx
    "A1 ????????"     // push esi
    "33C5"            // push edi
    "50"              // mov eax,dword ptr ds:[1F16980]
    "8D45 F4"         // xor eax,ebp
};

}  // namespace

namespace {
UObject* bl1_load_package_detour(const UObject* outer, const wchar_t* name, uint32_t flags) {
    LOG(INFO, L"[LOAD_PACKAGE] ~ {:p}, '{}', {:#010x}", (void*)outer, name, flags);
    return load_package_ptr(outer, name, flags);
}
}  // namespace

void BL1Hook::find_load_package(void) {
    if (env::defined(KEY_LOG_LOAD_PACKAGE)) {
        detour(LOAD_PACKAGE_PATTERN, &bl1_load_package_detour, &load_package_ptr,
               "bl1_load_package_detour");
    } else {
        load_package_ptr = LOAD_PACKAGE_PATTERN.sigscan_nullable<load_package_func>();
    }
    LOG(MISC, "LoadPackage: {:p}", reinterpret_cast<void*>(load_package_ptr));
}

[[nodiscard]] UObject* BL1Hook::load_package(const std::wstring& name, uint32_t flags) const {
    return load_package_ptr(nullptr, name.data(), flags);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
