#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/game/bl4/bl4.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/multi_sigscan.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2 && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

const constexpr Pattern<15> GOBJECTS_SIG{
    "44 8B 05 {????????}"  // mov r8d, [Borderlands4.exe+C5CD8F8]
    "8B 15 ????????"       // mov edx, [Borderlands4.exe+C5CD91C]
    "89 F9"                // mov ecx, edi
};

GObjects gobjects_wrapper{};

}  // namespace
namespace bl4 {
constinit MultiPattern gobjects_multi{GOBJECTS_SIG};
}  // namespace bl4

void BL4Hook::find_gobjects(void) {
    auto gobjects_sig = GOBJECTS_SIG.sigscan_nullable();
    LOG(MISC, "GObjects sig: {:p}", reinterpret_cast<void*>(gobjects_sig));
    auto gobjects_ptr = read_offset<GObjects::internal_type>(gobjects_sig);
    LOG(MISC, "GObjects: {:p}", reinterpret_cast<void*>(gobjects_ptr));

    gobjects_wrapper = GObjects(gobjects_ptr);

    // wait for gobjects to be initialized
    while (gobjects_ptr->ObjObjects.Count == 0) {
        const constexpr auto sleep_time = std::chrono::milliseconds{50};
        std::this_thread::sleep_for(sleep_time);
    }

    LOG(MISC, "GObjObjects at {}", gobjects_ptr->ObjObjects.Count);
}

const unreal::GObjects& BL4Hook::gobjects(void) const {
    return gobjects_wrapper;
}

}  // namespace unrealsdk::game

#endif
