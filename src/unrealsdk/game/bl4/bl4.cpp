#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl4/bl4.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/multi_sigscan.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/unrealsdk.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2 && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;
using namespace unrealsdk::memory;

namespace unrealsdk::game {
void BL4Hook::hook(void) {
    hook_antidebug();
    hook_process_event_and_wait_for_unpack();

    multi_sigscan(                         //
        &bl4::gnatives_multi,              //
        &bl4::ftexthistory_vftable_multi,  //
        &bl4::fnamepool_multi,             //
        &bl4::fname_find_or_store_multi,   //
        &bl4::gobjects_multi,              //
        &bl4::call_function_multi,         //
        &bl4::process_event_multi,         //
        &bl4::gmalloc_multi,               //
        &bl4::get_obj_path_name_multi,     //
        &bl4::get_field_path_name_multi,   //
        &bl4::construct_obj_multi,         //
        &bl4::find_obj_multi,              //
        &bl4::load_package_multi           //
    );

    // The exe is quite big, a couple of these funcs use delay loops, and we seem to have run into
    // timing issues before, so multithread the sigscans
    std::vector<std::thread> threads;
    for (auto func : {
             hook_call_function,
             find_fname_funcs,
             find_gmalloc,
             find_get_path_name,
             find_construct_object,
             find_static_find_object,
             find_load_package,
             find_fframe_step,
             find_ftext_as_culture_invariant,
             find_gobjects,
         }) {
        threads.emplace_back(func);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

void BL4Hook::post_init(void) {
    inject_console();
}

#pragma region FFrame::Step

namespace {

using gnatives_func = void (*)(UObject* obj, FFrame* stack, void* param);

gnatives_func* gnatives_table_ptr;

const constexpr Pattern<22> GNATIVES_PTR{
    "48 89 FA"             // mov rdx, rdi
    "4C 8D 0D {????????}"  // lea r9, [Borderlands4.exe+C5CBDB0]
    "41 FF 14 C1"          // call qword ptr [r9+rax*8]
    "48 83 C4 ??"          // add rsp, 20
    "80 7D F8 01"          // cmp byte ptr [rbp-08], 01
};

}  // namespace
namespace bl4 {
constinit MultiPattern gnatives_multi{GNATIVES_PTR};
}

void BL4Hook::find_fframe_step(void) {
    auto gnative_inst = GNATIVES_PTR.sigscan("gnative");
    LOG(MISC, "GNatives sig: {:p}", reinterpret_cast<void*>(gnative_inst));
    gnatives_table_ptr = read_offset<decltype(gnatives_table_ptr)>(gnative_inst);
    LOG(MISC, "GNatives: {:p}", reinterpret_cast<void*>(gnatives_table_ptr));
}

void BL4Hook::fframe_step(FFrame* frame, UObject* obj, void* param) const {
    auto curr_native = *frame->Code();
    frame->Code()++;
    gnatives_table_ptr[curr_native](obj, frame, param);
}

#pragma endregion

#pragma region FText::AsCultureInvariant

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
    uint32_t unknown2_minus_one = (uint32_t)-1;
    uint32_t unknown3_minus_one = (uint32_t)-1;
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
constinit MultiPattern ftexthistory_vftable_multi{FTEXTHISTORY_BASE_VFTABLE_PATTERN};
}

void BL4Hook::find_ftext_as_culture_invariant(void) {
    auto sig = FTEXTHISTORY_BASE_VFTABLE_PATTERN.sigscan_nullable();
    LOG(MISC, "FTextHistory_Base vftable sig: {:p}", reinterpret_cast<void*>(sig));

    ftexthistory_base_vftable = read_offset<uintptr_t*>(sig);
    LOG(MISC, "FTextHistory_Base vftable: {:p}",
        reinterpret_cast<void*>(ftexthistory_base_vftable));
}

void BL4Hook::ftext_as_culture_invariant(unreal::FText* text, std::wstring_view str) const {
    auto* base = unrealsdk::u_malloc<FTextHistory_Base>(sizeof(FTextHistory_Base));
    base->vftable = ftexthistory_base_vftable;
    base->ref_count = 1;  // pre-increment the ref count
    base->unknown0 = 0;
    base->unknown1 = 0;
    base->unknown2_minus_one = (uint32_t)-1;
    base->unknown3_minus_one = (uint32_t)-1;
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

}  // namespace unrealsdk::game

#endif
