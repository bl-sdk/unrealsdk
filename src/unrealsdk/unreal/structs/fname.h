#ifndef UNREALSDK_UNREAL_STRUCTS_FNAME_H
#define UNREALSDK_UNREAL_STRUCTS_FNAME_H

#include "unrealsdk/pch.h"

namespace unrealsdk::game {

class BL1Hook;
class BL1EHook;
class BL2Hook;
class BL3Hook;
class BL4Hook;

}  // namespace unrealsdk::game

namespace unrealsdk::unreal {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

struct FName {
   private:
    friend class game::BL1Hook;
    friend class game::BL1EHook;
    friend class game::BL2Hook;
    friend class game::BL3Hook;
    friend class game::BL4Hook;

    uint32_t index{0};
    uint32_t number{0};

   public:
    /**
     * @brief Construct a new FName object.
     * @note Automatically converts utf8 to utf16 if needed.
     *
     * @param index The name index to use.
     * @param name A string to lookup/create the name index of. Must be null terminated.
     * @param number The name number to use.
     */
    FName(void) = default;
    FName(uint32_t index, uint32_t number);
    explicit FName(const wchar_t* name, uint32_t number = 0);
    explicit FName(const std::string& name, uint32_t number = 0);
    explicit FName(const std::wstring& name, uint32_t number = 0);

    bool operator==(const FName& other) const;
    bool operator!=(const FName& other) const;

    /**
     * @brief Push the FName onto a stream in it's string representation.
     * @note Automatically converts utf8 to utf16 (or vice versa), as needed.
     *
     * @param stream The stream to push it onto.
     * @param name The FName to push.
     * @return The stream after pushing the FName.
     */
    friend std::ostream& operator<<(std::ostream& stream, const FName& name);
    friend std::wostream& operator<<(std::wostream& stream, const FName& name);

    /**
     * @brief Convert the FName to it's string representation.
     * @note Automatically converts utf8 to utf16 (or vice versa), as needed.
     *
     * @return The converted string.
     */
    operator std::string() const;
    operator std::wstring() const;
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

/**
 * @brief Concatenates a string and an FName.
 *
 * @param str The string to add to.
 * @param name The name to add.
 * @return The new string.
 */
inline std::string operator+(std::string& str, const unrealsdk::unreal::FName& name) {
    return str + std::string{name};
}
inline std::wstring operator+(std::wstring& str, const unrealsdk::unreal::FName& name) {
    return str + std::wstring{name};
}
inline std::string operator+(std::string_view str, const unrealsdk::unreal::FName& name) {
    return std::string{str} + std::string{name};
}
inline std::wstring operator+(std::wstring_view str, const unrealsdk::unreal::FName& name) {
    return std::wstring{str} + std::wstring{name};
}

/**
 * @brief Construct an FName literal from a wide string.
 *
 * @param str The string to create a name of.
 * @param len The length of the string.
 */
FName operator""_fn(const wchar_t* str, size_t len);

}  // namespace unrealsdk::unreal

// Custom FName formatter, which just casts to a string first
template <>
struct std::formatter<unrealsdk::unreal::FName> : std::formatter<std::string> {
    auto format(unrealsdk::unreal::FName name, std::format_context& ctx) const {
        return formatter<std::string>::format((std::string)name, ctx);
    }
};

namespace std {

// Custom FName hash function, which hashes as if it's a uint64
template <>
struct hash<unrealsdk::unreal::FName> {
    size_t operator()(const unrealsdk::unreal::FName& name) const {
        static_assert(sizeof(unrealsdk::unreal::FName) == sizeof(uint64_t),
                      "FName is not same size as a uint64");
        uint64_t val{};
        memcpy(&val, &name, sizeof(name));
        return hash<uint64_t>()(val);
    }
};

}  // namespace std

#endif /* UNREALSDK_UNREAL_STRUCTS_FNAME_H */
