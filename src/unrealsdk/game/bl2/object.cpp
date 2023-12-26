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
    construct_obj_ptr = CONSTRUCT_OBJECT_PATTERN.sigscan<construct_obj_func>();
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

#pragma region PathName

std::wstring BL2Hook::uobject_path_name(const UObject* obj) const {
    static UFunction* pathname_func = nullptr;

    // Optimize so we only call find once
    if (pathname_func == nullptr) {
        pathname_func = obj->Class->find_func_and_validate(L"PathName"_fn);
    }

    // Bound functions need mutable references, since they might actually modify the object
    // Object properties need mutable references, since you may want to modify the object you get
    // We know when calling PathName neither of these apply, so we want this function to explicitly
    // take a const reference - meaning we need a cast
    // While this is technically undefined behaviour, we only pass the reference around, so this
    // should be safe enough

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto mutable_obj = const_cast<UObject*>(obj);

    // The hook manager calls this function to work out if to run a hook, so we need to inject next
    // call to avoid recursion
    hook_manager::inject_next_call();
    return BoundFunction{pathname_func, mutable_obj}.call<UStrProperty, UObjectProperty>(
        mutable_obj);
}

#pragma endregion

#pragma region FindObject

UObject* BL2Hook::find_object(UClass* cls, const std::wstring& name) const {
    static UFunction* findobject_func = nullptr;

    if (findobject_func == nullptr) {
        findobject_func = cls->find_func_and_validate(L"FindObject"_fn);
    }

    return BoundFunction{findobject_func, cls}.call<UObjectProperty, UStrProperty, UClassProperty>(
        name, cls);
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
    load_package_ptr = LOAD_PACKAGE_PATTERN.sigscan<load_package_func>();
    LOG(MISC, "LoadPackage: {:p}", reinterpret_cast<void*>(load_package_ptr));
}

[[nodiscard]] UObject* BL2Hook::load_package(const std::wstring& name, uint32_t flags) const {
    return load_package_ptr(nullptr, name.data(), flags);
}

#pragma endregion

}  // namespace unrealsdk::game

#endif
