#include "pch.h"

#include "unreal/structs/fstring.h"
#include "unreal/structs/tarray.h"
#include "unreal/structs/tarray_funcs.h"
#include "unrealsdk.h"
#include "utils.h"

namespace unrealsdk::unreal {

using size_type = decltype(TArray<wchar_t>::count);

/**
 * @brief Gets the size of an stl string, ensuring it's valid to fit into a TArray.
 *
 * @param str The string to get the size of.
 * @return The size of the string.
 */
static size_type valid_size(const std::wstring& str) {
    auto size = str.size() + 1;  // Include the null terminator
    if (size > std::numeric_limits<size_type>::max()) {
        throw std::length_error("Tried to allocate a string longer than TArray max capacity!");
    }
    return static_cast<size_type>(size);
}

#pragma region TemporaryFString

TemporaryFString::TemporaryFString(const std::wstring& str)
    : TArray{str.c_str(), valid_size(str), valid_size(str)} {}

#pragma endregion

#pragma region UnmanagedFString

UnmanagedFString::UnmanagedFString(decltype(data) data, decltype(count) count, decltype(max) max)
    : TArray{data, count, max} {}
UnmanagedFString::UnmanagedFString(const std::string& str)
    : UnmanagedFString(unrealsdk::utils::widen(str)) {}
UnmanagedFString::UnmanagedFString(const std::wstring& str) : TArray{nullptr, 0, 0} {
    auto size = valid_size(str);
    this->resize(size);
    memcpy(this->data, str.c_str(), size * sizeof(*this->data));
}
UnmanagedFString::UnmanagedFString(UnmanagedFString&& other) noexcept
    : TArray{std::exchange(other.data, nullptr), std::exchange(other.count, 0),
             std::exchange(other.max, 0)} {}

UnmanagedFString& UnmanagedFString::operator=(const std::string& str) {
    return *this = UnmanagedFString{str};
}
UnmanagedFString& UnmanagedFString::operator=(const std::wstring& str) {
    return *this = UnmanagedFString{str};
}
UnmanagedFString& UnmanagedFString::operator=(UnmanagedFString&& other) noexcept {
    std::swap(this->data, other.data);
    std::swap(this->count, other.count);
    std::swap(this->max, other.max);
    return *this;
}

UnmanagedFString::operator std::string() const {
    return unrealsdk::utils::narrow(this->operator std::wstring());
}
UnmanagedFString::operator std::wstring() const {
    auto size = this->size();
    if (size == 0) {
        return L"";
    }

    // Remove a single trailing null, if necessary
    if (this->data[size - 1] == L'\0') {
        size--;
    }

    return {this->data, size};
}

#pragma endregion

#pragma region ManagedFString

ManagedFString::~ManagedFString() {
    if (this->data != nullptr) {
        unrealsdk::u_free(this->data);
        this->data = nullptr;
    }
}

ManagedFString::ManagedFString(const UnmanagedFString& other)
    : UnmanagedFString{unrealsdk::u_malloc<wchar_t>(other.max), other.count, other.max} {
    memcpy(this->data, other.data, this->count * sizeof(*this->data));
}

ManagedFString& ManagedFString::operator=(const UnmanagedFString& other) {
    this->resize(other.count);
    memcpy(this->data, other.data, this->count * sizeof(*this->data));
    return *this;
}

#pragma endregion

}  // namespace unrealsdk::unreal
