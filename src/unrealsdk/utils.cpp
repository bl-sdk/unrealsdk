#include "unrealsdk/pch.h"

#include "unrealsdk/utils.h"

namespace unrealsdk::utils {

// NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t is different size to char16_t");

std::string narrow(std::wstring_view wstr) {
    if (wstr.empty()) {
        return {};
    }

    auto num_chars = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()),
                                         nullptr, 0, nullptr, nullptr);
    std::string ret(num_chars, '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), ret.data(),
                            num_chars, nullptr, nullptr)
        != num_chars) {
        throw std::runtime_error("Failed to convert utf16 string!");
    }

    return ret;
}

std::wstring widen(std::string_view str) {
    if (str.empty()) {
        return {};
    }

    auto num_chars =
        MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring ret(num_chars, '\0');

    if (MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), ret.data(),
                            num_chars)
        != num_chars) {
        throw std::runtime_error("Failed to convert utf8 string!");
    }

    return ret;
}

std::filesystem::path get_this_dll_dir(void) {
    HMODULE this_module = nullptr;
    if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&get_this_dll_dir), &this_module)
        == 0) {
        // On error, better to return an empty path, i.e. the cwd
        // This is called to get the path of the log file, so if we threw we wouldn't have anything
        // to log the error to
        return {};
    }

    char buf[MAX_PATH];
    if (GetModuleFileNameA(this_module, &buf[0], sizeof(buf)) == 0) {
        return {};
    }

    return std::filesystem::path{buf}.parent_path();
}

// NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

}  // namespace unrealsdk::utils
