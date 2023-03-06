#include "pch.h"

#include "game/bl2/bl2.h"
#include "memory.h"
#include "unreal/wrappers/gnames.h"
#include "unreal/wrappers/gobjects.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

static GObjects gobjects_wrapper{};

void BL2Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x8B\x40\x08",
                                      "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gobjects_instr = sigscan(GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: {:p}", reinterpret_cast<void*>(gobjects_ptr));

    gobjects_wrapper = GObjects(gobjects_ptr);
}

const GObjects& BL2Hook::gobjects(void) const {
    return gobjects_wrapper;
};

static GNames gnames_wrapper;

void BL2Hook::find_gnames(void) const {
    static const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x83\x3C\x81\x00\x74\x5C",
                                    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto gnames_instr = sigscan(GNAMES_SIG);
    auto gnames_ptr = read_offset<GNames::internal_type>(gnames_instr);
    LOG(MISC, "GNames: {:p}", reinterpret_cast<void*>(gnames_ptr));

    gnames_wrapper = GNames(gnames_ptr);
}

const GNames& BL2Hook::gnames(void) const {
    return gnames_wrapper;
}

}  // namespace unrealsdk::game

#endif
