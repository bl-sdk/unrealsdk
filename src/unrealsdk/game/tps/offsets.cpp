#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets/ufield.h"
#include "unrealsdk/game/bl2/offsets/uobject.h"
#include "unrealsdk/game/tps/offsets/ustruct.h"
#include "unrealsdk/game/tps/tps.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offset_list.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal::offsets;

namespace unrealsdk::game {

namespace {

struct OffsetClasses {
    using UObject = bl2::UObject;
    using UField = bl2::UField;
    using UStruct = tps::UStruct;
};

const auto OFFSETS = OffsetList::from<OffsetClasses>();

}  // namespace

[[nodiscard]] const unreal::offsets::OffsetList& TPSHook::get_offsets(void) const {
    return OFFSETS;
}

}  // namespace unrealsdk::game
#endif
