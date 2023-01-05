#ifndef GAMES_BL2_H
#define GAMES_BL2_H

#include "pch.h"

#include "games/game_hook.h"
#include "sigscan.h"

using unrealsdk::sigscan::Pattern;

namespace unrealsdk::games {

class BL2Hook : public GameHook {
   protected:
    const Pattern gobjects_sig{"\x00\x00\x00\x00\x8B\x04\xB1\x8B\x40\x08",
                               "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF", 0};
    /**
     * @brief Get the signature for GNames.
     *
     * @return The signature pattern.
     */
    virtual Pattern get_gnames_sig(void);

    const Pattern fname_init_sig{
        "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x81\xEC\x9C\x0C",
        "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 0};
    // NOLINTNEXTLINE(modernize-use-using)  - need a typedef for the __thiscall
    typedef void*(__thiscall* fname_init_func)(unreal::FName* name,
                                               const wchar_t* str,
                                               int32_t number,
                                               int32_t find_type,
                                               int32_t split_name /*,
                                               int32_t unknown*/);
    void* fname_init_ptr;

   public:
    BL2Hook(void);
    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) override;
};

template <>
struct GameTraits<BL2Hook> {
    static constexpr auto NAME = "Borderlands 2";
    static constexpr bool IS_64BIT = false;
    static constexpr bool IS_UE4 = false;

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands2.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* GAMES_BL2_H */
