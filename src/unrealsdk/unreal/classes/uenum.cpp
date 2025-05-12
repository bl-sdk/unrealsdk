#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

std::unordered_map<FName, uint64_t> UEnum::get_names(void) const {
    auto bl3_enum = reinterpret_cast<const unrealsdk::game::bl3::UEnum*>(this);
    std::unordered_map<FName, uint64_t> output;
    for (size_t i = 0; i < bl3_enum->Names.size(); i++) {
        auto pair = bl3_enum->Names.at(i);
        output.emplace(pair.key, pair.value);
    }
    return output;
}

#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

std::unordered_map<FName, uint64_t> UEnum::get_names(void) const {
    auto bl2_enum = reinterpret_cast<const unrealsdk::game::bl2::UEnum*>(this);

    std::unordered_map<FName, uint64_t> output;
    for (size_t i = 0; i < bl2_enum->Names.size(); i++) {
        output.emplace(bl2_enum->Names.at(i), i);
    }
    return output;
}

#else
#error Unknown SDK flavour
#endif

}  // namespace unrealsdk::unreal
