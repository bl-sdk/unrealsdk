#include "unrealsdk/pch.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/game/bl4/bl4.h"
#include "unrealsdk/multi_sigscan.h"
#include "unrealsdk/unreal/alignment.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2 && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace unrealsdk::game {

namespace {

const constexpr Pattern<84> GMALLOC_PGO_SIG{
    // This is the inlined initialization code, it gets thousands of matches
    "48 8B 0D {????????}"      // mov rcx, [Borderlands4.exe+114F8EA0]
    "48 85 C9"                 // test rcx, rcx
    "75 ??"                    // jne Borderlands4.exe+E6DE28
    "8B 05 ????????"           // mov eax, [Borderlands4.exe+114D2468]
    "8B 0D ????????"           // mov ecx, [Borderlands4.AK::IAkStreamMgr::m_pStreamMgr+15AC]
    "65 4C 8B 04 25 ????????"  // mov r8, gs:[00000058]
    "49 8B 0C C8"              // mov rcx, [r8+rcx*8]
    "3B 81 ????????"           // cmp eax, [rcx+00000110]
    "7F 1D"                    // jg Borderlands4.exe+E6DE3E
    "48 8B 0D ????????"        // mov rcx, [Borderlands4.exe+114F8EA0]
    "48 8B 01"                 // mov rax, [rcx]
    "48 8B 40 48"              // mov rax, [rax+48]
    "48 83 C4 20"              // add rsp, 20
    "5E"                       // pop rsi
    "48 FF E0"                 // jmp rax
    // Not entirely sure this is still part of initalization, adding just to try get an 89 for the
    // multi sigscan
    "90"                 // nop
    "48 83 C4 ??"        // add rsp, 20h
    "5E"                 // pop rsi
    "C3"                 // retn
    "48 8D 0D ????????"  // lea rcx, cs:1514D2468h
    "48 89 D6"           // mov rsi, rdx
};
const constexpr Pattern<52> GMALLOC_NON_PGO_SIG{
    "48 8B 0D {????????}"  // mov rcx, [Borderlands4.exe+C4DBF30]
    "48 85 C9"             // test rcx, rcx
    "74 ??"                // je Borderlands4.exe+1D843
    "48 8B 01"             // mov rax, [rcx]
    "48 8B 40 ??"          // mov rax, [rax+28]
    "48 89 FA"             // mov rdx, rdi
    "41 89 F0"             // mov r8d, esi
    "48 83 C4 ??"          // add rsp, 28
    "5F"                   // pop rdi
    "5E"                   // pop rsi
    "48 FF E0"             // jmp rax
    "E8 ????????"          // call Borderlands4.exe+5B4E598
    "48 8B 0D ????????"    // mov rcx, [Borderlands4.exe+C4DBF30]
    "EB ??"                // jmp Borderlands4.exe+1D82D
    "CC"                   // int 3
    "48 89 C8"             // mov rax, rcx
};

struct FMalloc;
struct FMallocVFtable {
    uintptr_t unknown00;
    uintptr_t unknown08;
    uintptr_t unknown10;
    uintptr_t unknown18;
    uintptr_t unknown20;
    void* (*u_malloc)(FMalloc* self, uint64_t len, uint32_t align);
    uintptr_t unknown30;
    void* (*u_realloc)(FMalloc* self, void* original, uint64_t len, uint32_t align);
    uintptr_t unknown40;
    void* (*u_free)(FMalloc* self, void* data);
    uintptr_t unknown50;
    uintptr_t unknown58;
    uintptr_t possibly_get_allocation_size;
};
struct FMalloc {
    FMallocVFtable* vftable;
};

FMalloc* gmalloc;

}  // namespace
namespace bl4 {
constinit MultiPattern gmalloc_pgo_multi{GMALLOC_PGO_SIG};
constinit MultiPattern gmalloc_non_pgo_multi{GMALLOC_NON_PGO_SIG};
}  // namespace bl4

void BL4Hook::find_gmalloc(void) {
    auto gmalloc_sig =
        BL4Hook::choose_pattern(bl4::gmalloc_pgo_multi, bl4::gmalloc_non_pgo_multi, "GMalloc sig");
    volatile auto gmalloc_ptr = read_offset<FMalloc**>(gmalloc_sig);
    while (*gmalloc_ptr == nullptr) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::this_thread::sleep_for(std::chrono::milliseconds{50});
    }

    gmalloc = *gmalloc_ptr;
    LOG(MISC, "GMalloc: {:p}", reinterpret_cast<void*>(gmalloc));
}

void* BL4Hook::u_malloc(size_t len) const {
    auto ret = gmalloc->vftable->u_malloc(gmalloc, len, get_malloc_alignment(len));
    memset(ret, 0, len);
    return ret;
}
void* BL4Hook::u_realloc(void* original, size_t len) const {
    return gmalloc->vftable->u_realloc(gmalloc, original, len, get_malloc_alignment(len));
}
void BL4Hook::u_free(void* data) const {
    gmalloc->vftable->u_free(gmalloc, data);
}

}  // namespace unrealsdk::game

#endif
