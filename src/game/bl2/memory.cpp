#include "pch.h"

#include "game/bl2/bl2.h"
#include "memory.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

struct FMalloc;
struct FMallocVFtable {
    void* exec;
    void*(__thiscall* malloc)(FMalloc* self, uint32_t len, uint32_t align);
    void*(__thiscall* realloc)(FMalloc* self, void* original, uint32_t len, uint32_t align);
    void*(__thiscall* free)(FMalloc* self, void* data);
};
struct FMalloc {
    FMallocVFtable* vftable;
};

static FMalloc* gmalloc;

void BL2Hook::find_gmalloc(void) {
    static const Pattern GMALLOC_PATTERN{"\x00\x00\x00\x00\xFF\xD7\x83\xC4\x04\x89\x45\xE4",
                                         "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto sig_address = sigscan(GMALLOC_PATTERN);
    gmalloc = *read_offset<FMalloc**>(sig_address);
    LOG(MISC, "GMalloc: 0x%p", gmalloc);
}
void* BL2Hook::malloc(size_t len) const {
    auto ret = gmalloc->vftable->malloc(gmalloc, len, get_alignment(len));
    memset(ret, 0, len);
    return ret;
}
void* BL2Hook::realloc(void* original, size_t len) const {
    return gmalloc->vftable->realloc(gmalloc, original, len, get_alignment(len));
}
void BL2Hook::free(void* data) const {
    gmalloc->vftable->free(gmalloc, data);
}

}  // namespace unrealsdk::game

#endif
