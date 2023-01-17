#include "pch.h"

#include "utils.h"

namespace unrealsdk::utils {

// NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t is different size to char16_t");

std::string narrow(const std::wstring& wstr) {
    if (wstr.empty()) {
        return {};
    }

    auto num_chars =
        WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(wstr.c_str()),
                            static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);

    char* str = reinterpret_cast<char*>(malloc((num_chars + 1) * sizeof(char)));
    if (str == nullptr) {
        throw std::runtime_error("Failed to convert utf16 string!");
    }

    WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(wstr.c_str()),
                        static_cast<int>(wstr.size()), str, num_chars, nullptr, nullptr);
    str[num_chars] = L'\0';

    std::string ret{str};
    free(str);

    return ret;
}

std::wstring widen(const std::string& str) {
    if (str.empty()) {
        return {};
    }

    auto num_chars =
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    auto wstr = reinterpret_cast<wchar_t*>(malloc((num_chars + 1) * sizeof(wchar_t)));
    if (wstr == nullptr) {
        throw std::runtime_error("Failed to convert utf8 string!");
    }

    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions)
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), reinterpret_cast<wchar_t*>(wstr),
                        num_chars);
    wstr[num_chars] = L'\0';

    std::wstring ret{wstr};
    free(wstr);

    return ret;
}

// NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

}  // namespace unrealsdk::utils
