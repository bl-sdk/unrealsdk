#include "unreal/structs/fname.h"
#include "unreal/structs/gnames.h"
#include "unrealsdk.h"
#include "utils.h"

namespace unrealsdk::unreal {

FName::FName(int32_t index, int32_t number) : index(index), number(number) {}

FName::FName(const std::string& name, int32_t number) : FName(utils::widen(name), number){};
FName::FName(const std::wstring& name, int32_t number) : index(0), number(0) {
    unrealsdk::game->fname_init(this, name, number);
}

bool FName::operator==(const FName& other) const {
    return this->index == other.index && this->number == other.number;
}

bool FName::operator!=(const FName& other) const {
    return !operator==(other);
}

FName::operator std::string() const {
    auto entry = unrealsdk::game->gnames.at(this->index);

    std::string str;
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    if (entry->is_wide()) {
        str = utils::narrow(entry->WideName);
    } else {
        str = entry->AnsiName;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

    if (this->number != 0) {
        str += "_" + std::to_string(this->number - 1);
    }
    return str;
}

FName::operator std::wstring() const {
    auto entry = unrealsdk::game->gnames.at(this->index);

    std::wstring str;
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    if (entry->is_wide()) {
        str = entry->WideName;
    } else {
        str = utils::widen(entry->AnsiName);
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

    if (this->number != 0) {
        str += L"_" + std::to_wstring(this->number - 1);
    }
    return str;
}

}  // namespace unrealsdk::unreal
