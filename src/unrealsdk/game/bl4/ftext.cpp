#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/game/bl4/bl4.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/multi_sigscan.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unrealsdk.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2 && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

#pragma region PGO version

namespace {

// FTextHistory::AsCultureInvariant seems to get entirely inlined, replicate it

// binfold finds a couple of FTextHistory_Base funcs, xrefs to find table, xrefs and pick any
// This is the entire initializer, this sig gets hundreds of matches
const constexpr Pattern<46> FTEXTHISTORY_BASE_VFTABLE_PATTERN{
    "B9 30000000"           // mov ecx, 00000030
    "E8 ????????"           // call Borderlands4.exe+B17F7A0        <--- malloc wrapper
    "0F 57 C0"              // xorps xmm0, xmm0
    "0F 29 00"              // movaps [rax], xmm0
    "48 8D 0D ????????"     // lea rcx, [Borderlands4.exe+EC372D0]  <--- FTextHistory_Base vftable
    "48 89 08"              // mov [rax], rcx
    "48 C7 40 10 FFFFFFFF"  // mov qword ptr [rax+10], FFFFFFFFFFFFFFFF
    "0F 11 40 18"           // movups [rax+18], xmm0
    "48 C7 40 28 00000000"  // mov qword ptr [rax+28], 00000000
    ,
    19};

// NOLINTNEXTLINE(readability-identifier-naming)
struct FTextHistory_Base {
    uintptr_t* vftable;  // = vftable
    std::atomic<uint32_t> ref_count = 0;
    uint16_t unknown0 = 0;
    uint16_t unknown1 = 0;
    uint32_t unknown2_minus_one = static_cast<uint32_t>(-1);
    uint32_t unknown3_minus_one = static_cast<uint32_t>(-1);
    UnmanagedFString str;  // = 0
    // This one's weird. Initialiser suggests u32+4 padding, usage suggests pointer.
    uintptr_t unknown4_alt_str_ptr = 0;
};
// NOLINTBEGIN(readability-magic-numbers)
static_assert(sizeof(FTextHistory_Base) == 0x30);
static_assert(sizeof(FText) == 0x10);
// NOLINTEND(readability-magic-numbers)

uintptr_t* ftexthistory_base_vftable = nullptr;

}  // namespace
namespace bl4 {
constinit MultiPattern ftexthistory_vftable_pgo_multi{FTEXTHISTORY_BASE_VFTABLE_PATTERN};
}

void BL4Hook::ftext_as_culture_invariant_pgo(unreal::FText* text, std::wstring_view str) {
    auto* base = unrealsdk::u_malloc<FTextHistory_Base>(sizeof(FTextHistory_Base));
    base->vftable = ftexthistory_base_vftable;
    base->ref_count = 1;  // pre-increment the ref count
    base->unknown0 = 0;
    base->unknown1 = 0;
    base->unknown2_minus_one = static_cast<uint32_t>(-1);
    base->unknown3_minus_one = static_cast<uint32_t>(-1);
    base->str = str;
    base->unknown4_alt_str_ptr = 0;

    // Being slightly paranoid about the padding in ftext, zero it too
#if defined(__MINGW32__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnontrivial-memcall"
#endif
    memset(text, 0, sizeof(*text));
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#if defined(__MINGW32__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    text->data = reinterpret_cast<FTextData*>(base);
    text->flags = FText::FLAG_FROM_NAME_OR_STRING | FText::FLAG_INVARIANT_CULTURE;
}

#pragma endregion

#pragma region Non-PGO version
namespace {

using ftext_as_culture_invariant_func = void (*)(FText* self, const wchar_t* str);
ftext_as_culture_invariant_func ftext_as_culture_invariant_ptr;

const constexpr Pattern<30> FTEXT_AS_CULTURE_INVARIANT_PATTERN{
    "56"                 // push rsi
    "48 83 EC ??"        // sub rsp, 40
    "48 89 CE"           // mov rsi, rcx
    "48 8B 05 ????????"  // mov rax, [Borderlands4.exe+C372940]
    "48 31 E0"           // xor rax, rsp
    "48 89 44 24 ??"     // mov [rsp+38], rax
    "48 89 54 24 ??"     // mov [rsp+28], rdx
    "31 C0"              // xor eax, eax
};

}  // namespace
namespace bl4 {
constinit MultiPattern ftext_as_culture_invariant_non_pgo_multi{FTEXT_AS_CULTURE_INVARIANT_PATTERN};
}

void BL4Hook::ftext_as_culture_invariant_non_pgo(unreal::FText* text, std::wstring_view str) {
    const std::wstring with_null{str};
    ftext_as_culture_invariant_ptr(text, with_null.data());
}

#pragma endregion

void BL4Hook::find_ftext_as_culture_invariant(void) {
    auto pgo_addr = bl4::ftexthistory_vftable_pgo_multi.addr();
    auto non_pgo_addr = bl4::ftext_as_culture_invariant_non_pgo_multi.addr();
    LOG(MISC, "FTextHistory_Base vftable sig: {:p}", reinterpret_cast<void*>(pgo_addr));
    LOG(MISC, "FText::AsCultureInvariant: {:p}", reinterpret_cast<void*>(non_pgo_addr));

    if (pgo_addr != 0) {
        ftexthistory_base_vftable = read_offset<uintptr_t*>(pgo_addr);
        LOG(MISC, "FTextHistory_Base vftable: {:p}",
            reinterpret_cast<void*>(ftexthistory_base_vftable));
    } else {
        ftext_as_culture_invariant_ptr =
            reinterpret_cast<ftext_as_culture_invariant_func>(non_pgo_addr);
    }
}

void BL4Hook::ftext_as_culture_invariant(unreal::FText* text, std::wstring_view str) const {
    if (ftexthistory_base_vftable == nullptr) {
        BL4Hook::ftext_as_culture_invariant_non_pgo(text, str);
    } else {
        BL4Hook::ftext_as_culture_invariant_pgo(text, str);
    }
}

}  // namespace unrealsdk::game

#endif
