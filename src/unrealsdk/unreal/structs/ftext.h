#ifndef UNREALSDK_UNREAL_STRUCTS_FTEXT_H
#define UNREALSDK_UNREAL_STRUCTS_FTEXT_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/tsharedpointer.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

struct FTextData {
    uintptr_t* vftable;
};

struct FText {
   private:
    static const constexpr auto FLAG_TRANSIENT = 1 << 0;
    static const constexpr auto FLAG_INVARIANT_CULTURE = 1 << 1;
    static const constexpr auto FLAG_FROM_NAME_OR_STRING = 1 << 4;

    TSharedPointer<FTextData> data;
    uint32_t flags;

   public:
    /**
     * @brief Construct a new FText using a given string.
     * @note Automatically converts utf8 to utf16 (or vice versa) if needed.
     *
     * @param str The string to set it to. Always uses invariant culture.
     */
    FText() = default;
    FText(std::string_view str);
    FText(std::wstring_view str);

    /**
     * @brief Assigns a new value to the FText.
     * @note Automatically converts utf8 to utf16 (or vice versa) if needed.
     *
     * @param str The string to assign.
     * @return A reference to this FText.
     */
    FText& operator=(std::string_view str);
    FText& operator=(std::wstring_view str);

    /**
     * @brief Convert the FText to it's string representation.
     * @note Automatically converts utf8 to utf16 (or vice versa), as needed.
     *
     * @return The converted string.
     */
    operator std::string() const;
    operator std::wstring() const;
    operator std::wstring_view() const;

    /**
     * @brief Destroys the FText.
     */
    ~FText();

    // Encourage not storing FTexts (should use std strings) by deleting all of these.
    FText(const FText&) = delete;
    FText(FText&&) = delete;
    FText& operator=(const FText&) = delete;
    FText& operator=(FText&&) = delete;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

// Custom FText formatter, which just casts to a string first
template <>
struct unrealsdk::fmt::formatter<unrealsdk::unreal::FText>
    : unrealsdk::fmt::formatter<std::string> {
    auto format(unrealsdk::unreal::FText text, unrealsdk::fmt::format_context& ctx) const {
        return formatter<std::string>::format((std::string)text, ctx);
    }
};

#endif /* UNREALSDK_UNREAL_STRUCTS_FTEXT_H */
