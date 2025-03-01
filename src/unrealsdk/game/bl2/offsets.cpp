#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/game/bl2/offsets/uobject.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offset_list.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal::offsets;

namespace unrealsdk::game {

namespace {

struct OffsetClasses {
    using UObject = bl2::UObject;
    using UField = generic::UField<bl2::UObject>;
};

const auto OFFSETS = OffsetList::from<OffsetClasses>();

}  // namespace

[[nodiscard]] const unreal::offsets::OffsetList& BL2Hook::get_offsets(void) const {
    return OFFSETS;
}

}  // namespace unrealsdk::game
#endif
