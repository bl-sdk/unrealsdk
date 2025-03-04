#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl1/bl1.h"
#include "unrealsdk/game/bl2/offsets/ufield.h"
#include "unrealsdk/game/bl2/offsets/uobject.h"
#include "unrealsdk/game/bl2/offsets/uproperty.h"
#include "unrealsdk/game/bl2/offsets/ustruct.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offset_list.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal::offsets;

namespace unrealsdk::game {

namespace {

struct OffsetClasses {
    // TODO
    using UObject = bl2::UObject;
    using UField = bl2::UField;
    using UStruct = bl2::UStruct;
    using UProperty = bl2::UProperty;
};

const auto OFFSETS = OffsetList::from<OffsetClasses>();

}  // namespace

[[nodiscard]] const unreal::offsets::OffsetList& BL1Hook::get_offsets(void) const {
    return OFFSETS;
}

}  // namespace unrealsdk::game
#endif
