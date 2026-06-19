#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::unreal {

FName::FName(uint32_t index, uint32_t number) : index(index), number(number) {}

FName::FName(const wchar_t* name, uint32_t number) {
    unrealsdk::internal::fname_init(this, name, number);
}
FName::FName(const std::string& name, uint32_t number) : FName(utils::widen(name), number) {};
FName::FName(const std::wstring& name, uint32_t number) {
    unrealsdk::internal::fname_init(this, name, number);
}

bool FName::operator==(const FName& other) const {
    return this->index == other.index && this->number == other.number;
}

bool FName::operator!=(const FName& other) const {
    return !operator==(other);
}

std::ostream& operator<<(std::ostream& stream, const FName& name) {
    auto variant = unrealsdk::internal::fname_get_str(name);
    if (std::holds_alternative<const std::string_view>(variant)) {
        stream << std::get<const std::string_view>(variant);
    } else {
        stream << utils::narrow(std::get<const std::wstring_view>(variant));
    }

    if (name.number != 0) {
        stream << '_' << std::to_string(name.number - 1);
    }
    return stream;
}

std::wostream& operator<<(std::wostream& stream, const FName& name) {
    auto variant = unrealsdk::internal::fname_get_str(name);
    if (std::holds_alternative<const std::wstring_view>(variant)) {
        stream << std::get<const std::wstring_view>(variant);
    } else {
        stream << utils::widen(std::get<const std::string_view>(variant));
    }

    if (name.number != 0) {
        stream << '_' << std::to_wstring(name.number - 1);
    }
    return stream;
}

FName::operator std::string() const {
    std::ostringstream stream;
    stream << *this;
    return stream.str();
}
FName::operator std::wstring() const {
    std::wostringstream stream;
    stream << *this;
    return stream.str();
}

FName operator""_fn(const wchar_t* str, size_t /*len*/) {
    return FName{str};
}

}  // namespace unrealsdk::unreal
