#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/structs/tpair.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

std::unordered_map<FName, uint64_t> UEnum::get_names(void) const {
    auto names = unrealsdk::internal::uenum_get_names(this);

    std::unordered_map<FName, uint64_t> output;
    for (size_t i = 0; i < names.size(); i++) {
        auto pair = names.at(i);
        output.emplace(pair.key, pair.value);
    }
    return output;
}

}  // namespace unrealsdk::unreal
