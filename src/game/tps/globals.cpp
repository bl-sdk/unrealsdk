#include "pch.h"

#include "game/bl2/bl2.h"
#include "game/tps/tps.h"
#include "memory.h"
#include "unreal/wrappers/gnames.h"

#if defined(UE3) && defined(ARCH_X86)

namespace unrealsdk::game {

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

static GNames gnames_wrapper;

void TPSHook::find_gnames(void) const {
    static const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x5E\x5D\xC3\x8B\x15",
                                    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 0};

    auto gnames_instr = sigscan(GNAMES_SIG);
    auto gnames_ptr = read_offset<GNames::internal_type>(gnames_instr);
    LOG(MISC, "GNames: {:p}", reinterpret_cast<void*>(gnames_ptr));

    gnames_wrapper = GNames(gnames_ptr);
}

const GNames& TPSHook::gnames(void) const {
    return gnames_wrapper;
}

}  // namespace unrealsdk::game

#endif
