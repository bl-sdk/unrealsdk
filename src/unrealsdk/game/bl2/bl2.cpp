#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer_funcs.h"
#include "unrealsdk/unrealsdk.h"

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

    inject_console();

    hexedit_set_command();
    hexedit_array_limit();
    hexedit_array_limit_message();
}

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

}  // namespace unrealsdk::game

#endif
