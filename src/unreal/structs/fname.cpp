#include "unreal/structs/fname.h"
#include "unrealsdk.h"

namespace unrealsdk::unreal {

FName::FName(int32_t index, int32_t number) : index(index), number(number) {}

FName::FName(const std::wstring& name, int32_t number) : index(0), number(0) {
    unrealsdk::game->fname_init(this, name, number);
}

bool FName::operator==(const FName& other) const {
    return this->index == other.index && this->number == other.number;
}

bool FName::operator!=(const FName& other) const {
    return !operator==(other);
}

}  // namespace unrealsdk::unreal
