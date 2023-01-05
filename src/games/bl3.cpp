#include "pch.h"

#include "games/bl3.h"
#include "games/game_hook.h"
#include "sigscan.h"
#include "unreal/structs/fname.h"
#include "unreal/wrappers/gobjects.h"

using namespace unrealsdk::sigscan;

namespace unrealsdk::games {

void BL3Hook::find_gobjects(void) {
    static const Pattern GOBJECTS_SIG{
        "\x48\x8D\x0D\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\xE8\x00\x00\x00\x00\xC6\x05",
        "\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF", 3};

    auto gobjects_instr = scan(start, size, GOBJECTS_SIG);
    auto gobjects_ptr = read_offset<unreal::GObjects::internal_type>(gobjects_instr);
    LOG(MISC, "GObjects: 0x%p", gobjects_ptr);

    this->gobjects = unreal::GObjects(gobjects_ptr);
}

void BL3Hook::find_gnames(void) {
    static const Pattern GNAMES_SIG{
        "\xE8\x00\x00\x00\x00\x48\x00\x00\x48\x89\x1D\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83"
        "\xC4\x28\xC3\x00\xDB\x48\x89\x1D\x00\x00\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x83\xC4"
        "\x00\xC3",
        "\xFF\x00\x00\x00\x00\xFF\x00\x00\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF"
        "\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF"
        "\x00\xFF",
        0xB};

    auto gnames_instr = scan(start, size, GNAMES_SIG);
    auto gnames_ptr = *read_offset<unreal::GNames::internal_type*>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    this->gnames = unreal::GNames(gnames_ptr);
}

void BL3Hook::find_fname_init(void) {
    static const Pattern FNAME_INIT_PATTERN{
        "\x40\x53\x48\x83\xEC\x30\xC7\x44\x24\x00\x00\x00\x00\x00",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00"};

    this->fname_init_ptr = scan<fname_init_func*>(start, size, FNAME_INIT_PATTERN);
    LOG(MISC, "FNameInit: 0x%p", this->fname_init_ptr);
}

void BL3Hook::fname_init(unreal::FName* name, const std::wstring& str, int32_t number) {
    *name = this->fname_init_ptr(str.c_str(), number, 1);
}

}  // namespace unrealsdk::games
