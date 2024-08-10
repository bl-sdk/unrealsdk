#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/unreal/wrappers/gnames.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

GObjects gobjects_wrapper{};

const constinit Pattern<17> GOBJECTS_SIG{
    "8B 0D {????????}"  // mov ecx, [Borderlands2.exe+1682BD0]
    "8B 04 ??"          // mov eax, [ecx+esi*4]
    "8B 40 ??"          // mov eax, [eax+08]
    "25 00020000"       // and eax, 00000200
};

}  // namespace

void BL2Hook::find_gobjects(void) {
    auto gobjects_ptr = read_offset<GObjects::internal_type>(GOBJECTS_SIG.sigscan_nullable());
    LOG(MISC, "GObjects: {:p}", reinterpret_cast<void*>(gobjects_ptr));

    gobjects_wrapper = GObjects(gobjects_ptr);
}

const GObjects& BL2Hook::gobjects(void) const {
    return gobjects_wrapper;
};

namespace {

GNames gnames_wrapper{};

const constinit Pattern<10> GNAMES_SIG{
    "A3 {????????}"  // mov [BorderlandsPreSequel.exe+1520214], eax
    "8B 45 ??"       // mov eax, [ebp+10]
    "89 03"          // mov [ebx], eax
};

}  // namespace

void BL2Hook::find_gnames(void) {
    auto gnames_ptr = read_offset<GNames::internal_type>(GNAMES_SIG.sigscan_nullable());
    LOG(MISC, "GNames: {:p}", reinterpret_cast<void*>(gnames_ptr));

    gnames_wrapper = GNames(gnames_ptr);
}

const GNames& BL2Hook::gnames(void) const {
    return gnames_wrapper;
}

}  // namespace unrealsdk::game

#endif
