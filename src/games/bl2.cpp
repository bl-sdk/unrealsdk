#include "pch.h"

#include "games/bl2.h"
#include "games/game_hook.h"
#include "sigscan.h"
#include "unreal/wrappers/gobjects.h"

using namespace unrealsdk::sigscan;

namespace unrealsdk::games {

Pattern BL2Hook::get_gnames_sig(void) {
    return {"\x00\x00\x00\x00\x83\x3C\x81\x00\x74\x5C", "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF",
            0};
}

BL2Hook::BL2Hook(void) {
    auto [start, size] = get_exe_range();

    {
        auto gobjects_instr = scan(start, size, this->gobjects_sig);
        auto gobjects_ptr = read_offset<unreal::GObjects::internal_type>(gobjects_instr);
        LOG(MISC, "GObjects: 0x%p", gobjects_ptr);
        this->gobjects = unreal::GObjects(gobjects_ptr);
    }

    {
        auto gnames_instr = scan(start, size, this->get_gnames_sig());
        auto gnames_ptr = read_offset<unreal::GNames::internal_type>(gnames_instr);
        LOG(MISC, "GNames: 0x%p", gnames_ptr);
        this->gnames = unreal::GNames(gnames_ptr);
    }

    {
        this->fname_init_ptr = scan<void*>(start, size, this->fname_init_sig);
        LOG(MISC, "FNameInit: 0x%p", this->fname_init_ptr);
    }
}

void BL2Hook::fname_init(unreal::FName* name, const std::wstring& str, int32_t number) {
    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str.c_str(), number, 1,
                                                            1);  //, 0);
}

}  // namespace unrealsdk::games
