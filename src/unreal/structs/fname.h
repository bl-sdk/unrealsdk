#ifndef UNREAL_STRUCTS_FNAME_H
#define UNREAL_STRUCTS_FNAME_H

#include "pch.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

struct FName {
   private:
    int32_t index;
    int32_t number;

   public:
    /**
     * @brief Construct a new FName object.
     * @note Automatically converts utf8 to utf16 if needed.
     *
     * @param index The name index to use.
     * @param name A string to lookup/create the name index of.
     * @param number The name number to use.
     */
    FName(int32_t index, int32_t number);
    FName(const std::string& name, int32_t number = 0);
    FName(const std::wstring& name, int32_t number = 0);

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

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

/**
 * @brief Construct an FName literal from a wide string.
 *
 * @param str The string to create a name of.
 * @param len The length of the string.
 */
FName operator"" _fn(const wchar_t* str, size_t len);

}  // namespace unrealsdk::unreal

// Custom FName formatter, which just casts to a string first
template <>
struct unrealsdk::fmt::formatter<unrealsdk::unreal::FName>
    : unrealsdk::fmt::formatter<std::string> {
    auto format(unrealsdk::unreal::FName name, unrealsdk::fmt::format_context& ctx) {
        return formatter<std::string>::format((std::string)name, ctx);
    }
};

#endif /* UNREAL_STRUCTS_FNAME_H */
