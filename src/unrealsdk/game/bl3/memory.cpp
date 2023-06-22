#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/alignment.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

using fmemory_malloc_func = void* (*)(uint64_t len, uint32_t align);
using fmemory_realloc_func = void* (*)(void* original, uint64_t len, uint32_t align);
using fmemory_free_func = void (*)(void* data);
fmemory_malloc_func fmemory_malloc_ptr;
fmemory_realloc_func fmemory_realloc_ptr;
fmemory_free_func fmemory_free_ptr;

const constinit Pattern<25> MALLOC_PATTERN{
    "48 89 5C 24 ??"     // mov [rsp+08], rbx
    "57"                 // push rdi
    "48 83 EC 20"        // sub rsp, 20
    "48 8B F9"           // mov rdi, rcx
    "8B DA"              // mov ebx, edx
    "48 8B 0D ????????"  // mov rcx, [Borderlands3.exe+68C4E08]
    "48 85 C9"           // test rcx, rcx
};

const constinit Pattern<31> REALLOC_PATTERN{
    "48 89 5C 24 ??"     // mov [rsp+08], rbx
    "48 89 74 24 ??"     // mov [rsp+10], rsi
    "57"                 // push rdi
    "48 83 EC 20"        // sub rsp, 20
    "48 8B F1"           // mov rsi, rcx
    "41 8B D8"           // mov ebx, r8d
    "48 8B 0D ????????"  // mov rcx, [Borderlands3.exe+68C4E08]
    "48 8B FA"           // mov rdi, rdx
};

const constinit Pattern<20> FREE_PATTERN{
    "48 85 C9"           // test rcx, rcx
    "74 ??"              // je Borderlands3.exe+15D37E3
    "53"                 // push rbx
    "48 83 EC 20"        // sub rsp, 20
    "48 8B D9"           // mov rbx, rcx
    "48 8B 0D ????????"  // mov rcx, [Borderlands3.exe+68C4E08]
};

}  // namespace

void BL3Hook::find_gmalloc(void) {
    fmemory_malloc_ptr = MALLOC_PATTERN.sigscan<fmemory_malloc_func>();
    fmemory_realloc_ptr = REALLOC_PATTERN.sigscan<fmemory_realloc_func>();
    fmemory_free_ptr = FREE_PATTERN.sigscan<fmemory_free_func>();

    LOG(MISC, "FMemory::Malloc: {:p}", reinterpret_cast<void*>(fmemory_malloc_ptr));
    LOG(MISC, "FMemory::Realloc: {:p}", reinterpret_cast<void*>(fmemory_realloc_ptr));
    LOG(MISC, "FMemory::Free: {:p}", reinterpret_cast<void*>(fmemory_free_ptr));
}
void* BL3Hook::u_malloc(size_t len) const {
    auto ret = fmemory_malloc_ptr(len, get_malloc_alignment(len));
    memset(ret, 0, len);
    return ret;
}
void* BL3Hook::u_realloc(void* original, size_t len) const {
    return fmemory_realloc_ptr(original, len, get_malloc_alignment(len));
}
void BL3Hook::u_free(void* data) const {
    fmemory_free_ptr(data);
}

}  // namespace unrealsdk::game

#endif
