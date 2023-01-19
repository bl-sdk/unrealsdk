#include "pch.h"

#if defined(UE3) && defined(ARCH_X86)

#include "game/tps_aodk.h"
#include "memory.h"

using namespace unrealsdk::memory;

namespace unrealsdk::game {

void TPSAoDKHook::hexedit_array_limit_message(void) {
    static const Pattern ARRAY_LIMIT_MESSAGE{"\x7C\x7B\x8B\x8D\x94\xEE\xFF\xFF",
                                             "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto array_limit_msg = sigscan<uint8_t*>(ARRAY_LIMIT_MESSAGE);
    if (array_limit_msg == nullptr) {
        LOG(MISC, "Couldn't find array limit message signature, assuming already hex edited");
    } else {
        LOG(MISC, "Array Limit Message: 0x%p", array_limit_msg);

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit_msg, 1);
        array_limit_msg[0] = 0x75;
        // NOLINTEND(readability-magic-numbers)
    }
}

void TPSAoDKHook::find_gnames(void) {
    static const Pattern GNAMES_SIG{"\x00\x00\x00\x00\x8B\x04\xB1\x5E\x5D\xC3\x8B\x15",
                                    "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 0};

    auto gnames_instr = sigscan(GNAMES_SIG);
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

}  // namespace unrealsdk::game

#endif
