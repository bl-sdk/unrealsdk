#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl3/offsets.h"
#include "unrealsdk/game/bl3/bl3.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/structs/tarray_funcs.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal::offsets;

namespace unrealsdk::game {
namespace bl3 {
namespace {

constexpr auto OFFSETS = OFFSET_LIST_FROM_NAMESPACE();

}

unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> UEnum::get_names(void) const {
    unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> output{};
    output.resize(this->Names.size());

    // We know this is flat so we can straight memcpy
    memcpy(output.data, this->Names.data, this->Names.count * sizeof(*this->Names.data));

    return output;
}

}  // namespace bl3

[[nodiscard]] const unreal::offsets::OffsetList& BL3Hook::get_offsets(void) const {
    return bl3::OFFSETS;
}

unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> BL3Hook::uenum_get_names(
    const unreal::UEnum* uenum) const {
    return reinterpret_cast<const bl3::UEnum*>(uenum)->get_names();
}

}  // namespace unrealsdk::game
#endif
