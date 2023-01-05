#ifndef GAMES_BL3_H
#define GAMES_BL3_H

#include "pch.h"

#include "games/game_hook.h"
#include "sigscan.h"
#include "unreal/structs/fname.h"

using unrealsdk::sigscan::Pattern;

namespace unrealsdk::games {

class BL3Hook : public GameHook {
   protected:
    const Pattern gobjects_sig{
        "\x48\x8D\x0D\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\xE8\x00\x00\x00\x00\xC6\x05",
        "\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF", 3};
    const Pattern gnames_sig{
        "\xE8\x00\x00\x00\x00\x48\x00\x00\x48\x89\x1D\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83"
        "\xC4\x28\xC3\x00\xDB\x48\x89\x1D\x00\x00\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83\xC4"
        "\x00\xC3",
        "\xFF\x00\x00\x00\x00\xFF\x00\x00\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF"
        "\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF"
        "\x00\xFF",
        0xB};

    const Pattern fname_init_sig{"\x40\x53\x48\x83\xEC\x30\xC7\x44\x24\x00\x00\x00\x00\x00",
                                 "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00", 0};
    using fname_init_func = unreal::FName(const wchar_t* str, int32_t number, int32_t find_type);
    fname_init_func* fname_init_ptr;

   public:
    BL3Hook();
    void fname_init(unreal::FName* name, const std::wstring& str, int32_t number) override;
};

template <>
struct GameTraits<BL3Hook> {
    static constexpr auto NAME = "Borderlands 3";
    static constexpr bool IS_64BIT = true;
    static constexpr bool IS_UE4 = true;

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands3.exe";
    }
};

}  // namespace unrealsdk::games

#endif /* GAMES_BL3_H */
