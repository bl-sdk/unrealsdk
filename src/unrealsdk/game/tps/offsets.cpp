#include "unrealsdk/pch.h"
#include "unrealsdk/game/tps/offsets.h"
#include "unrealsdk/game/tps/tps.h"
#include "unrealsdk/unreal/offset_list.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal::offsets;

namespace unrealsdk::game {
namespace tps {
namespace {

constexpr auto OFFSETS = OFFSET_LIST_FROM_NAMESPACE();

}
}  // namespace tps

[[nodiscard]] const unreal::offsets::OffsetList& TPSHook::get_offsets(void) const {
    return tps::OFFSETS;
}

}  // namespace unrealsdk::game
#endif
