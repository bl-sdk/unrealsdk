#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl4/bl4.h"
#include "unrealsdk/game/bl4/offsets.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/multi_sigscan.h"
#include "unrealsdk/unreal/structs/gnames.h"
#include "unrealsdk/utils.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2 && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

const constexpr Pattern<37> FNAMEPOOL_PGO_SIG{
    "75 ??"              // jne Borderlands4.exe+1158B33
    "48 8D 05 ????????"  // lea rax, [Borderlands4.exe+1150CD40]        <--- FNamePool
    "48 89 CE"           // mov rsi, rcx
    "48 89 C1"           // mov rcx, rax
    "89 D7"              // mov edi, edx
    "E8 ????????"        // call Borderlands4.exe+114D380               <--- Init func
    "89 FA"              // mov edx, edi
    "48 89 F1"           // mov rcx, rsi
    "C6 05 ???????? 01"  // mov byte ptr [Borderlands4.exe+1150CD30], 1 <--- Initialized flag
    "83 FA 01"           // cmp edx, 1
};
const constexpr auto FNAMEPOOL_PGO_PTR_OFFSET = 5;
const constexpr auto FNAMEPOOL_PGO_INITIALIZED_OFFSET = 29;

const constexpr Pattern<34> FNAMEPOOL_NON_PGO_SIG{
    "48 89 CE"           // mov rsi, rcx
    "80 3D ???????? 00"  // cmp cs:byte_14C4E7C08, 0
    "75 ??"              // jne Borderlands4.exe+1B158
    "48 8D 0D ????????"  // lea rcx, [Borderlands4.exe+C4E7C40]          <--- FNamePool
    "E8 ????????"        // call Borderlands4.exe+3172E                  <--- Init func
    "C6 05 ???????? 01"  // mov byte ptr [Borderlands4.exe+C4E7C08], 01  <--- Initialized flag
    "83 FF 01"           // cmp edi, 01
};
const constexpr auto FNAMEPOOL_NON_PGO_PTR_OFFSET = 15;
const constexpr auto FNAMEPOOL_NON_PGO_INITIALIZED_OFFSET = 26;

FNamePool* name_pool_ptr = nullptr;

// Search for the string `ERROR_NAME_SIZE_EXCEEDED`, check refs, should only be one. It is *not*
// this function that directly points at, need to go one more ref out, this time there are a lot
// more. We're looking for one that takes a string view, which includes some logic comparing against
// '_'/95/0x5F (to split the name). Remember it's wchar-indexed.
const constexpr Pattern<39> FNAME_FIND_OR_STORE_WSTRING_PGO{
    "41 57"                 // push r15
    "41 56"                 // push r14
    "56"                    // push rsi
    "57"                    // push rdi
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000440
    "48 89 CE"              // mov rsi, rcx
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+11399940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000438], rax
    "48 63 42 08"           // movsxd rax, dword ptr [rdx+08]
};
const constexpr Pattern<35> FNAME_FIND_OR_STORE_WSTRING_NON_PGO{
    "56"                    // push rsi
    "57"                    // push rdi
    "53"                    // push rbx
    "48 81 EC ????????"     // sub rsp, 00000440
    "48 89 CE"              // mov rsi, rcx
    "48 8B 05 ????????"     // mov rax, [Borderlands4.exe+C372940]
    "48 31 E0"              // xor rax, rsp
    "48 89 84 24 ????????"  // mov [rsp+00000438], rax
    "48 63 42 08"           // movsxd rax, dword ptr [rdx+08]
};

struct FNameStringView {
    const wchar_t* str;
    uint32_t len;
    uint32_t non_ascii;
};

using fname_find_or_store_wstring_func = void (*)(FName* self,
                                                  const FNameStringView* str,
                                                  uint32_t find_name);
fname_find_or_store_wstring_func fname_find_or_store_wstring_ptr;

}  // namespace
namespace bl4 {
constinit MultiPattern fnamepool_pgo_multi{FNAMEPOOL_PGO_SIG};
constinit MultiPattern fnamepool_non_pgo_multi{FNAMEPOOL_NON_PGO_SIG};
constinit MultiPattern fname_find_or_store_pgo_multi{FNAME_FIND_OR_STORE_WSTRING_PGO};
constinit MultiPattern fname_find_or_store_non_pgo_multi{FNAME_FIND_OR_STORE_WSTRING_NON_PGO};
}  // namespace bl4

void BL4Hook::find_fname_funcs(void) {
    fname_find_or_store_wstring_ptr = BL4Hook::choose_pattern<fname_find_or_store_wstring_func>(
        bl4::fname_find_or_store_pgo_multi, bl4::fname_find_or_store_non_pgo_multi,
        "FNameHelper::FindOrStoreWString");

    auto pgo_addr = bl4::fnamepool_pgo_multi.addr();
    auto non_pgo_addr = bl4::fnamepool_non_pgo_multi.addr();
    LOG(MISC, "FNamePool sig: {:p}/{:p}", reinterpret_cast<void*>(pgo_addr),
        reinterpret_cast<void*>(non_pgo_addr));

    auto name_pool_base = pgo_addr == 0 ? non_pgo_addr : pgo_addr;
    auto ptr_offset = pgo_addr == 0 ? FNAMEPOOL_NON_PGO_PTR_OFFSET : FNAMEPOOL_PGO_PTR_OFFSET;
    auto initialized_offset =
        pgo_addr == 0 ? FNAMEPOOL_NON_PGO_INITIALIZED_OFFSET : FNAMEPOOL_PGO_INITIALIZED_OFFSET;

    name_pool_ptr = read_offset<decltype(name_pool_ptr)>(name_pool_base + ptr_offset);
    LOG(MISC, "FNamePool: {:p}", reinterpret_cast<void*>(name_pool_ptr));

    // I tried manually initaliaing the pool, but it didn't seem to like it
    // Just wait for it to happen on the main thread instead
    auto name_pool_initialized =
        read_offset<volatile uint8_t*>(name_pool_base + initialized_offset);
    while (*name_pool_initialized != 0) {
        const constexpr auto sleep_time = std::chrono::milliseconds{50};
        std::this_thread::sleep_for(sleep_time);
    }
}

std::variant<const std::string_view, const std::wstring_view> BL4Hook::fname_get_str(
    const unreal::FName& name) const {
    auto entry = reinterpret_cast<bl4::FNameEntry*>(name_pool_ptr->at(name.index));
    const size_t size = entry->Metadata >> bl4::FNameEntry::META_SIZE_BIT_OFFSET;

    if ((entry->Metadata & FNameEntry::NAME_WIDE_MASK) != 0) {
        return std::wstring_view{&entry->Name.Wide[0], size};
    }
    return std::string_view{&entry->Name.Ansi[0], size};
}

void BL4Hook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    const constexpr auto max_ascii_char = L'\x7F';
    const constexpr auto fname_add = 1;

    const std::wstring_view wstr_view{str};
    const FNameStringView fname_view{
        .str = str,
        .len = static_cast<uint32_t>(wstr_view.size()),
        .non_ascii =
            std::ranges::any_of(wstr_view, [](wchar_t chr) { return chr > max_ascii_char; }) ? 1
                                                                                             : 0U,
    };
    fname_find_or_store_wstring_ptr(name, &fname_view, fname_add);

    if (number != 0) {
        name->number = number;
    }
}

}  // namespace unrealsdk::game

#endif
