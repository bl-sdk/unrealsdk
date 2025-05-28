#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"

#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/wrappers/gnames.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

GObjects gobjects_wrapper{};

const constinit Pattern<18> GOBJECTS_SIG{
    "8B 0D {????????}"  // mov ecx, [01FB4DD8]
    "8B 04 ??"          // mov eax, [ecx+esi*4]
    "8B 50 ??"          // mov edx, [eax+0C]
    "21 58 ??"          // and [eax+08], ebx
    "89 50 ??"          // mov [eax+0C], edx
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

namespace {

GNames gnames_wrapper{};

const constinit Pattern<21> GNAMES_SIG{
    "A1 {????????}"  // mov eax, [01FB4DA8]
    "8B 0C ??"       // mov ecx, [eax+esi*4]
    "68 ????????"    // push 00001000
    "6A ??"          // push 00
    "E8 ????????"    // call 005C21F0
    "5E"             // pop esi
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
