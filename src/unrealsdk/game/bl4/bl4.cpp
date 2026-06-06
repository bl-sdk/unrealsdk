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

    multi_sigscan(                                       //
        &bl4::gnatives_multi,                            //
        &bl4::ftexthistory_vftable_pgo_multi,            //
        &bl4::ftext_as_culture_invariant_non_pgo_multi,  //
        &bl4::fnamepool_pgo_multi,                       //
        &bl4::fnamepool_non_pgo_multi,                   //
        &bl4::fname_find_or_store_pgo_multi,             //
        &bl4::fname_find_or_store_non_pgo_multi,         //
        &bl4::gobjects_multi,                            //
        &bl4::call_function_multi,                       //
        &bl4::process_event_multi,                       //
        &bl4::gmalloc_pgo_multi,                         //
        &bl4::gmalloc_non_pgo_multi,                     //
        &bl4::get_obj_path_name_pgo_multi,               //
        &bl4::get_obj_path_name_non_pgo_multi,           //
        &bl4::get_field_path_name_pgo_multi,             //
        &bl4::get_field_path_name_non_pgo_multi,         //
        &bl4::construct_obj_pgo_multi,                   //
        &bl4::construct_obj_non_pgo_multi,               //
        &bl4::find_obj_pgo_multi,                        //
        &bl4::find_obj_non_pgo_multi,                    //
        &bl4::load_package_multi                         //
    );

    hook_call_function();
    find_fname_funcs();
    find_gmalloc();
    find_get_path_name();
    find_construct_object();
    find_static_find_object();
    find_load_package();
    find_fframe_step();
    find_ftext_as_culture_invariant();
    find_gobjects();
}

void BL4Hook::post_init(void) {
    inject_console();
}

uintptr_t BL4Hook::choose_pattern(const memory::MultiPattern& pgo,
                                  const memory::MultiPattern& non_pgo,
                                  std::string_view name) {
    auto pgo_addr = pgo.addr();
    auto non_pgo_addr = non_pgo.addr();
    LOG(INFO, "{}: {:p}/{:p}", name, reinterpret_cast<void*>(pgo_addr),
        reinterpret_cast<void*>(non_pgo_addr));
    return pgo_addr == 0 ? non_pgo_addr : pgo_addr;
}

#pragma region FFrame::Step

namespace {

using gnatives_func = void (*)(UObject* obj, FFrame* stack, void* param);

gnatives_func* gnatives_table_ptr;

const constexpr Pattern<26> GNATIVES_PTR{
    "48 89 F?"             // mov rdx, rdi              | mov rdx, rsi
    "4C 8D 45 ??"          // lea r8, [rbp-08]
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

}  // namespace unrealsdk::game

#endif
