#include "pch.h"

#include "games/bl3.h"
#include "games/game_hook.h"
#include "sigscan.h"
#include "unreal/wrappers/gobjects.h"
#include "unreal/structs/fname.h"

using namespace unrealsdk::sigscan;

namespace unrealsdk::games {

BL3Hook::BL3Hook(void) {
    auto [start, size] = get_exe_range();

    {
        auto gobjects_instr = scan(start, size, this->gobjects_sig);
        auto gobjects_ptr = read_offset<unreal::GObjects::internal_type>(gobjects_instr);
        LOG(MISC, "GObjects: 0x%p", gobjects_ptr);
        this->gobjects = unreal::GObjects(gobjects_ptr);
    }

    {
        auto gnames_instr = scan(start, size, this->gnames_sig);
        auto gnames_ptr = *read_offset<unreal::GNames::internal_type*>(gnames_instr);
        LOG(MISC, "GNames: 0x%p", gnames_ptr);
        this->gnames = unreal::GNames(gnames_ptr);
    }

    {
        this->fname_init_ptr = scan<fname_init_func*>(start, size, this->fname_init_sig);
        LOG(MISC, "FNameInit: 0x%p", this->fname_init_ptr);
    }
}

void BL3Hook::fname_init(unreal::FName *name, const std::wstring &str, int32_t number) {
    *name = this->fname_init_ptr(str.c_str(), number, 1);
}


}  // namespace unrealsdk::games
