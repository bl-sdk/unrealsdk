#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/game/bl3/offsets/uobject.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offset_list.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal::offsets;

namespace unrealsdk::game {

namespace {

struct OffsetClasses {
    using UObject = bl3::UObject;
    using UField = generic::UField<bl3::UObject>;
};

const auto OFFSETS = OffsetList::from<OffsetClasses>();

}  // namespace

[[nodiscard]] const unreal::offsets::OffsetList& BL3Hook::get_offsets(void) const {
    return OFFSETS;
}

}  // namespace unrealsdk::game
#endif
