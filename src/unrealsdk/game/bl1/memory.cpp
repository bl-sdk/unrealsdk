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

FMalloc* gmalloc;

// - NOTE -
// This is *unique* to GMalloc for the 141 UDK however its not completely unique. Its just all
//  other usages are also GMalloc. Might be a better idea to use ds:[eax+8] since realloc is a bit
//  rarer and used pretty much exclusively by dynamic contiguous containers.
//

const constinit Pattern<20> GMALLOC_PATTERN{
    "8B0D {????????}"  // mov ecx,dword ptr ds:[1F73BB4]
    "8B01"             // mov eax,dword ptr ds:[ecx]
    "8B50 04"          // mov edx,dword ptr ds:[eax+4]
    "6A 08"            // push 8
    "68 E4000000"      // push E4
    "FFD2"             // call edx
};

}  // namespace

// ############################################################################//
//  | BL1Hook Methods |
// ############################################################################//

void BL1Hook::find_gmalloc(void) {
    // TODO: I would prefer to create FMalloc if it is null since all usages are wrapped in an
    // if null then create block. However, with the delayed hooking via the init function this will
    // always be non-null.

    gmalloc = *read_offset<FMalloc**>(GMALLOC_PATTERN.sigscan("GMalloc"));
    if (gmalloc == nullptr) {
        // This happens if we try to hook it too early.
        throw std::runtime_error("GMalloc is null");
    }
    LOG(MISC, "GMalloc: {:p}", reinterpret_cast<void*>(gmalloc));
}

void* BL1Hook::u_malloc(size_t len) const {
    auto ret = gmalloc->vftable->u_malloc(gmalloc, len, get_malloc_alignment(len));
    memset(ret, 0, len);
    return ret;
}

void* BL1Hook::u_realloc(void* original, size_t len) const {
    return gmalloc->vftable->u_realloc(gmalloc, original, len, get_malloc_alignment(len));
}

void BL1Hook::u_free(void* data) const {
    gmalloc->vftable->u_free(gmalloc, data);
}

}  // namespace unrealsdk::game

#endif
