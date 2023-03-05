#include "pch.h"

#include "game/bl3/bl3.h"
#include "memory.h"
#include "unreal/wrappers/gobjects.h"
#include "unreal/wrappers/gnames.h"

#if defined(UE4) && defined(ARCH_X64)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

static GObjects gobjects_wrapper{};

void BL3Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{
        "\x48\x8D\x0D\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\xE8\x00\x00\x00\x00\xC6\x05",
        "\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF", 3};

    auto gobjects_instr = sigscan(GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);

    gobjects_wrapper = GObjects(gobjects_ptr);
}

const GObjects& BL3Hook::gobjects(void) const {
    return gobjects_wrapper;
};

static GNames gnames_wrapper{};

void BL3Hook::find_gnames(void) {
    static const Pattern GNAMES_SIG{
        "\xE8\x00\x00\x00\x00\x48\x00\x00\x48\x89\x1D\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83"
        "\xC4\x28\xC3\x00\xDB\x48\x89\x1D\x00\x00\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83\xC4"
        "\x00\xC3",
        "\xFF\x00\x00\x00\x00\xFF\x00\x00\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF"
        "\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF"
        "\x00\xFF",
        0xB};

    auto gnames_instr = sigscan(GNAMES_SIG);
    auto gnames_ptr = *read_offset<GNames::internal_type*>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    gnames_wrapper = GNames(gnames_ptr);
}

const GNames& BL3Hook::gnames(void) const {
    return gnames_wrapper;
}

}  // namespace unrealsdk::game

#endif
