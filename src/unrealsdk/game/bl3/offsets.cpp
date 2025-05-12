#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl3/offsets.h"
#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offset_list.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal::offsets;

namespace unrealsdk::game {
namespace bl3 {
namespace {

constexpr auto OFFSETS = OFFSET_LIST_FROM_NAMESPACE();

}

}  // namespace bl3

[[nodiscard]] const unreal::offsets::OffsetList& BL3Hook::get_offsets(void) const {
    return bl3::OFFSETS;
}

}  // namespace unrealsdk::game
#endif
