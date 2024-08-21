#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/fweakobjectptr.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"
#include "unrealsdk/unreal/structs/tpersistentobjectptr.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

/*
Both assignment operators get inlined unfortunately, so we need to manually recreate them.

How they seem to work is:
1. Construct a new path using it's UObject constructor.
   This is difficult for us to replicate, hence using hooks to call it.
2. Move assign the new path into the pointer.
   This swaps the old path into our local.
3. Assign the weak pointer.
4. Copy the tag from a global.
5. Destroy the local path, which now holds the old one.

*/

namespace {

static_assert(std::atomic<int32_t>::is_always_lock_free
                  && sizeof(std::atomic<int32_t>) == sizeof(int32_t)
                  && alignof(std::atomic<int32_t>) == alignof(int32_t),
              "atomic int32_t soft object ptr tags might not be implemented in hardware");

const constinit Pattern<43> SET_SOFT_OBJ_PTR_PATTERN{
    "E8 ????????"          // call FSoftObjectPath::FSoftObjectPath
    "48 8B D0"             // mov rdx, rax
    "48 8D 4D ??"          // lea rcx, [rbp-20]
    "E8 ????????"          // call FSoftObjectPath::operator=   <-- Move assignment
    "48 8B D6"             // mov rdx, rsi
    "48 8D 4D D0"          // lea rcx, [rbp-30]
    "E8 ????????"          // call FWeakObjectPtr::operator=
    "33 C0"                // xor eax, eax
    "F0 0FB1 1D ????????"  // lock cmpxchg [FSoftObjectPath::CurrentTag], ebx
    "48 8B 4D ??"          // mov rcx, [rbp-20]
};

const constexpr auto SOFT_OBJ_PATH_CONSTRUCTOR_OFFSET = 1;
const constexpr auto SOFT_OBJ_PATH_CURRENT_TAG_OFFSET = 35;

using fsoftobjectpath_constructor_func = void (*)(FSoftObjectPath* self, const UObject* obj);
using fsoftobjectpath_tag_type = const std::atomic<int32_t>*;

fsoftobjectpath_constructor_func fsoftobjectpath_constructor_ptr;
fsoftobjectpath_tag_type fsoftobjectpath_tag_ptr;

const constinit Pattern<36> SET_LAZY_OBJ_PTR_PATTERN{
    "E8 ????????"          // call FLazyObjectPath::FLazyObjectPath
    "48 8B D6"             // mov rdx, rsi
    "48 8D 4C 24 ??"       // lea rcx, [rsp+30]
    "0F10 00"              // movups xmm0, [rax]        <--- Effectively FLazyObjectPath::operator=
    "0F11 44 24 ??"        // movups [rsp+3C], xmm0     <---
    "E8 ????????"          // call FWeakObjectPtr::operator=
    "33 C0"                // xor eax, eax
    "F0 0FB1 1D ????????"  // lock cmpxchg [FLazyObjectPath::CurrentTag], ebx
};

const constexpr auto LAZY_OBJ_PATH_CONSTRUCTOR_OFFSET = 1;
const constexpr auto LAZY_OBJ_PATH_CURRENT_TAG_OFFSET = 32;

using flazyobjectpath_constructor_func = void (*)(FLazyObjectPath* self, const UObject* obj);
using flazyobjectpath_tag_type = const std::atomic<int32_t>*;

flazyobjectpath_constructor_func flazyobjectpath_constructor_ptr;
flazyobjectpath_tag_type flazyobjectpath_tag_ptr;

}  // namespace

void BL3Hook::find_persistent_obj_ptrs(void) {
    auto set_soft_obj_ptr = SET_SOFT_OBJ_PTR_PATTERN.sigscan_nullable();

    fsoftobjectpath_constructor_ptr = read_offset<fsoftobjectpath_constructor_func>(
        set_soft_obj_ptr + SOFT_OBJ_PATH_CONSTRUCTOR_OFFSET);
    fsoftobjectpath_tag_ptr =
        read_offset<fsoftobjectpath_tag_type>(set_soft_obj_ptr + SOFT_OBJ_PATH_CURRENT_TAG_OFFSET);

    LOG(MISC, "FSoftObjectPath::FSoftObjectPath: {:p}",
        reinterpret_cast<void*>(fsoftobjectpath_constructor_ptr));
    LOG(MISC, "FSoftObjectPath::CurrentTag: {:p}",
        reinterpret_cast<const void*>(fsoftobjectpath_tag_ptr));

    auto set_lazy_obj_ptr = SET_LAZY_OBJ_PTR_PATTERN.sigscan_nullable();

    flazyobjectpath_constructor_ptr = read_offset<flazyobjectpath_constructor_func>(
        set_lazy_obj_ptr + LAZY_OBJ_PATH_CONSTRUCTOR_OFFSET);
    flazyobjectpath_tag_ptr =
        read_offset<flazyobjectpath_tag_type>(set_lazy_obj_ptr + LAZY_OBJ_PATH_CURRENT_TAG_OFFSET);

    LOG(MISC, "FLazyObjectPath::FLazyObjectPath: {:p}",
        reinterpret_cast<void*>(flazyobjectpath_constructor_ptr));
    LOG(MISC, "FLazyObjectPath::CurrentTag: {:p}",
        reinterpret_cast<const void*>(flazyobjectpath_tag_ptr));
}

void BL3Hook::fsoftobjectptr_assign(FSoftObjectPtr* ptr, const UObject* obj) const {
    FSoftObjectPath new_path{};
    fsoftobjectpath_constructor_ptr(&new_path, obj);

    std::swap(ptr->identifier, new_path);
    unrealsdk::gobjects().set_weak_object(&ptr->weak_ptr, obj);
    ptr->tag = fsoftobjectpath_tag_ptr->load();

    // Since we have an unmanaged fstring, need to manually free it
    static_assert(std::is_same_v<decltype(new_path.subpath), UnmanagedFString>);
    new_path.subpath.free();
}

void BL3Hook::flazyobjectptr_assign(FLazyObjectPtr* ptr, const UObject* obj) const {
    // Setting to null doesn't seem to be supported.
    // If we pass a null object, we end up creating a zero'd guid, which falls into an extra if
    // statement that calls `UObjectBaseUtility::MarkPackageDirty` *with the null object*
    // While we could trivially create our own zero'd guid, not sure if that has consequences, and
    // lazy properties aren't used much anyway, so better to throw
    if (obj != nullptr) {
        throw std::invalid_argument("Lazy object pointers cannot be set to null.");
    }

    FLazyObjectPath new_path{};
    flazyobjectpath_constructor_ptr(&new_path, obj);

    std::swap(ptr->identifier, new_path);
    unrealsdk::gobjects().set_weak_object(&ptr->weak_ptr, obj);
    ptr->tag = flazyobjectpath_tag_ptr->load();
}

}  // namespace unrealsdk::game

#endif
