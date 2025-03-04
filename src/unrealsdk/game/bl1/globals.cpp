#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"

#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/wrappers/gnames.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING) \
    && defined(UNREALSDK_GAME_BL1)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

// ############################################################################//
//  | GOBJECTS |
// ############################################################################//

GObjects gobjects_wrapper{};

const constinit Pattern<15> GOBJECTS_SIG{
    "8B0D {????????}"  // mov ecx,dword ptr ds:[1FB85A8]
    "8B04B1"           // mov eax,dword ptr ds:[ecx+esi*4]
    "8B50 0C"          // mov edx,dword ptr ds:[eax+C]
    "2158 08"          // and dword ptr ds:[eax+8],ebx
};

}  // namespace

void BL1Hook::find_gobjects(void) {
    auto gobjects_ptr = read_offset<GObjects::internal_type>(GOBJECTS_SIG.sigscan_nullable());
    LOG(MISC, "GObjects: {:p}", reinterpret_cast<void*>(gobjects_ptr));

    gobjects_wrapper = GObjects(gobjects_ptr);
}

const GObjects& BL1Hook::gobjects(void) const {
    return gobjects_wrapper;
};

// ############################################################################//
//  | GNAMES |
// ############################################################################//

namespace {

GNames gnames_wrapper{};

const constinit Pattern<15> GNAMES_SIG{
    "A1 {????????}"  // mov eax,dword ptr ds:[1FB8578]
    "8B0CB0"         // mov ecx,dword ptr ds:[eax+esi*4]
    "68 00100000"    // push 1000
    "6A 00"          // push 0
};

}  // namespace

void BL1Hook::find_gnames(void) {
    auto gnames_ptr = read_offset<GNames::internal_type>(GNAMES_SIG.sigscan_nullable());
    LOG(MISC, "GNames: {:p}", reinterpret_cast<void*>(gnames_ptr));

    gnames_wrapper = GNames(gnames_ptr);
}

const GNames& BL1Hook::gnames(void) const {
    return gnames_wrapper;
}

}  // namespace unrealsdk::game

#endif
