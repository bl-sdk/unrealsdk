#include "unrealsdk/pch.h"

#include "unrealsdk/game/aodk/aodk.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/wrappers/gnames.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

GNames gnames_wrapper;

const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x5E\x5D\xC3\x8B\x15",
                         "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 0};

}  // namespace

void AoDKHook::find_gnames(void) const {
    auto gnames_instr = sigscan(GNAMES_SIG);
    auto gnames_ptr = read_offset<GNames::internal_type>(gnames_instr);
    LOG(MISC, "GNames: {:p}", reinterpret_cast<void*>(gnames_ptr));

    gnames_wrapper = GNames(gnames_ptr);
}

const GNames& AoDKHook::gnames(void) const {
    return gnames_wrapper;
}

}  // namespace unrealsdk::game

#endif
