#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/alignment.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"  // thiscall on non-class
#endif

struct FMalloc;
struct FMallocVFtable {
    void* exec;
    void*(__thiscall* u_malloc)(FMalloc* self, uint32_t len, uint32_t align);
    void*(__thiscall* u_realloc)(FMalloc* self, void* original, uint32_t len, uint32_t align);
    void*(__thiscall* u_free)(FMalloc* self, void* data);
};
struct FMalloc {
    FMallocVFtable* vftable;
};

#if defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

FMalloc** gmalloc_ptr;

const constinit Pattern<8> GMALLOC_PATTERN{
    "89 0D {????????}"  // mov [01F703F4],ecx { (05AA6980) }
    "8B 11"             // mov edx,[ecx]
};

}  // namespace

void BL1Hook::find_gmalloc(void) {
    gmalloc_ptr = read_offset<FMalloc**>(GMALLOC_PATTERN.sigscan_nullable());
    LOG(MISC, "GMalloc*: {:p}", reinterpret_cast<void*>(gmalloc_ptr));

    if (gmalloc_ptr != nullptr) {
        LOG(MISC, "GMalloc: {:p}", reinterpret_cast<void*>(*gmalloc_ptr));
    }
}

void* BL1Hook::u_malloc(size_t len) const {
    if (gmalloc_ptr == nullptr) {
        throw std::runtime_error("tried allocating memory while gmalloc was still null!");
    }
    auto gmalloc = *gmalloc_ptr;

    auto ret = gmalloc->vftable->u_malloc(gmalloc, len, get_malloc_alignment(len));
    memset(ret, 0, len);
    return ret;
}

void* BL1Hook::u_realloc(void* original, size_t len) const {
    if (gmalloc_ptr == nullptr) {
        throw std::runtime_error("tried allocating memory while gmalloc was still null!");
    }
    auto gmalloc = *gmalloc_ptr;

    return gmalloc->vftable->u_realloc(gmalloc, original, len, get_malloc_alignment(len));
}

void BL1Hook::u_free(void* data) const {
    if (gmalloc_ptr == nullptr) {
        throw std::runtime_error("tried allocating memory while gmalloc was still null!");
    }
    auto gmalloc = *gmalloc_ptr;

    gmalloc->vftable->u_free(gmalloc, data);
}

}  // namespace unrealsdk::game

#endif
