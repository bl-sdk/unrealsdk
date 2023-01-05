#include "pch.h"

#include "games/tps_aodk.h"

namespace unrealsdk::games {

Pattern TPSAoDKHook::get_gnames_sig(void) {
    return {"\x00\x00\x00\x00\x8B\x04\xB1\x5E\x5D\xC3\x8B\x15",
                            "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 0};
}

void TPSAoDKHook::fname_init(unreal::FName* name, const std::wstring& str, int32_t number) {
    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str.c_str(), number, 1, 1);
}

}  // namespace unrealsdk::games
