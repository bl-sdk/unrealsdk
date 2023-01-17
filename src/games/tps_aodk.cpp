#include "pch.h"

#if defined(UE3) && defined(ARCH_X86)

#include "games/tps_aodk.h"
#include "sigscan.h"

using namespace unrealsdk::sigscan;

namespace unrealsdk::games {

void TPSAoDKHook::find_gnames(void) {
    static const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x5E\x5D\xC3\x8B\x15",
                                    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 0};

    auto gnames_instr = scan(start, size, GNAMES_SIG);
    auto gnames_ptr = read_offset<unreal::GNames::internal_type>(gnames_instr);
    LOG(MISC, "GNames: 0x%p", gnames_ptr);

    this->gnames = unreal::GNames(gnames_ptr);
}

void TPSAoDKHook::fname_init(unreal::FName* name, const std::wstring& str, int32_t number) const {
    this->fname_init(name, str.c_str(), number);
}
void TPSAoDKHook::fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const {
    // NOLINTNEXTLINE(modernize-use-using)
    typedef void*(__thiscall * fname_init_func)(unreal::FName * name, const wchar_t* str,
                                                int32_t number, int32_t find_type,
                                                int32_t split_name, int32_t /*unknown*/);

    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str, number, 1, 1, 0);
}

}  // namespace unrealsdk::games

#endif
