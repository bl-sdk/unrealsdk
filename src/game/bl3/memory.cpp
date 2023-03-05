#include "pch.h"

#include "game/bl3/bl3.h"
#include "memory.h"
#include "unreal/utils.h"

#if defined(UE4) && defined(ARCH_X64)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

using fmemory_malloc_func = void* (*)(uint64_t len, uint32_t align);
using fmemory_realloc_func = void* (*)(void* original, uint64_t len, uint32_t align);
using fmemory_free_func = void (*)(void* data);
static fmemory_malloc_func fmemory_malloc_ptr;
static fmemory_realloc_func fmemory_realloc_ptr;
static fmemory_free_func fmemory_free_ptr;

void BL3Hook::find_gmalloc(void) {
    static const Pattern MALLOC_PATTERN{
        "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xF9\x8B\xDA\x48\x8B\x0D\x00\x00\x00\x00"
        "\x48\x85\xC9",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
        "\xFF\xFF\xFF"};

    static const Pattern REALLOC_PATTERN{
        "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xF1\x41\x8B\xD8\x48"
        "\x8B\x0D\x00\x00\x00\x00\x48\x8B\xFA",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF"};

    static const Pattern FREE_PATTERN{
        "\x48\x85\xC9\x74\x00\x53\x48\x83\xEC\x20\x48\x8B\xD9\x48\x8B\x0D\x00\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00"};

    fmemory_malloc_ptr = sigscan<fmemory_malloc_func>(MALLOC_PATTERN);
    fmemory_realloc_ptr = sigscan<fmemory_realloc_func>(REALLOC_PATTERN);
    fmemory_free_ptr = sigscan<fmemory_free_func>(FREE_PATTERN);

    LOG(MISC, "FMemory::Malloc: 0x%p", fmemory_malloc_ptr);
    LOG(MISC, "FMemory::Realloc: 0x%p", fmemory_realloc_ptr);
    LOG(MISC, "FMemory::Free: 0x%p", fmemory_free_ptr);
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
