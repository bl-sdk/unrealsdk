#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/game/bl2/offsets.h"
#include "unrealsdk/game/bl3/offsets.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UEnum, UNREALSDK_UENUM_FIELDS);

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK

std::unordered_map<FName, uint64_t> UEnum::get_names(void) const {
    std::unordered_map<FName, uint64_t> output;
    auto names = this->Names();
    for (size_t i = 0; i < names.size(); i++) {
        auto pair = names.at(i);

        // Oak enums include the enum class name and a namespace separator before the value's name
        // If we see it, strip it
        const std::wstring str_key{pair.key};
        auto after_colons = str_key.find_first_not_of(L':', str_key.find_first_of(L':'));

        output.emplace(after_colons == std::string::npos ? pair.key : str_key.substr(after_colons),
                       pair.value);
    }
    return output;
}

#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW

std::unordered_map<FName, uint64_t> UEnum::get_names(void) const {
    std::unordered_map<FName, uint64_t> output;
    auto names = this->Names();
    for (size_t i = 0; i < names.size(); i++) {
        // Willow enums just use the raw name, and are always stored in order
        output.emplace(names.at(i), i);
    }
    return output;
}

#else
#error Unknown SDK flavour
#endif

}  // namespace unrealsdk::unreal
