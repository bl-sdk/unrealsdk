#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"
#include "unrealsdk/memory.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING) \
    && defined(UNREALSDK_GAME_BL1)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

// ############################################################################//
//  | SIGNATURES |
// ############################################################################//

namespace {

const constinit Pattern<17> SET_COMMAND_SIG{
    "75 16"        // jne borderlands.87E1A7
    "8D4C24 18"    // lea ecx,dword ptr ss:[esp+18]
    "68 ????????"  // push borderlands.1B18900
    "51"           // push ecx
    "E8 ????????"  // call <borderlands.sub_5C1E10>
};

const constinit Pattern<20> ARRAY_LIMIT_SIG{
    "6A 64"          // push 64
    "50"             // push eax
    "46"             // inc esi
    "E8 ????????"    // call <borderlands.sub_517770>
    "83C4 08"        // add esp,8
    "3BF0"           // cmp esi,eax
    "0F8C 59FFFFFF"  // jl borderlands.5E7D33
};

const constinit Pattern<29> ARRAY_LIMIT_MESSAGE{
    "0F8C 7E000000"  // jl borderlands.5E7E64
    "8B4C24 38"      // mov ecx,dword ptr ss:[esp+38]
    "83C7 9D"        // add edi,FFFFFF9D
    "57"             // push edi
    "68 ????????"    // push borderlands.1A7F42C
    "E8 ????????"    // call borderlands.51BAA0
    "E9 ????????"    // jmp borderlands.5E7E64
};

}  // namespace

// ############################################################################//
//  | HEX EDITS |
// ############################################################################//

void BL1Hook::hexedit_set_command(void) {
    uint8_t* set_command_msg = SET_COMMAND_SIG.sigscan_nullable<uint8_t*>();

    if (set_command_msg == nullptr) {
        LOG(ERROR, "Failed to find set command message signature.");
        return;
    }

    LOG(INFO, "Set Command: {:p}", reinterpret_cast<void*>(set_command_msg));

    // NOLINTBEGIN(readability-magic-numbers)
    unlock_range(set_command_msg, 2);
    set_command_msg[0] = 0x90;
    set_command_msg[1] = 0x90;
    // NOLINTEND(readability-magic-numbers)
}

void BL1Hook::hexedit_array_limit(void) {
    // - NOTE -
    // In BL2 this seems to be inlined however for BL1 its not so we will NOP the CALL and its
    // 4 byte address. The caller cleans the stack so need to worry about that. Might also be a
    // good idea to move the signature forward to the CALL so we don't need to index weirdly.
    //

    auto array_limit = ARRAY_LIMIT_SIG.sigscan_nullable<uint8_t*>();
    if (array_limit == nullptr) {
        LOG(ERROR, "Couldn't find array limit signature");
    } else {
        LOG(MISC, "Array Limit: {:p}", reinterpret_cast<void*>(array_limit));

        // NOLINTBEGIN(readability-magic-numbers)
        auto* call_instruction = array_limit + 4;

        // Should *never* be true
        if (call_instruction[0] != 0xE8) {
            LOG(ERROR, "[ARRAY_LIMIT] ~ Instruction at {:p} + 0x04 is {:02X}", (void*)array_limit,
                *call_instruction);
            return;
        }

        // Patching out the CALL ???????? to 5 NOP instructions
        unlock_range(call_instruction, 5);
        call_instruction[0] = 0x90;
        call_instruction[1] = 0x90;
        call_instruction[2] = 0x90;
        call_instruction[3] = 0x90;
        call_instruction[4] = 0x90;
        // NOLINTEND(readability-magic-numbers)
    }
}

void BL1Hook::hexedit_array_limit_message(void) const {
    uint8_t* array_limit_msg = ARRAY_LIMIT_MESSAGE.sigscan_nullable<uint8_t*>();

    if (array_limit_msg == nullptr) {
        LOG(ERROR, "Failed to find array limit message signature.");
        return;
    }

    LOG(MISC, "Array Limit Message: {:p}", reinterpret_cast<void*>(array_limit_msg));

    // NOLINTBEGIN(readability-magic-numbers)
    unlock_range(array_limit_msg, 6);
    array_limit_msg[0] = 0xE9;
    array_limit_msg[1] = 0x7F;
    array_limit_msg[2] = 0x00;
    array_limit_msg[3] = 0x00;
    array_limit_msg[4] = 0x00;
    array_limit_msg[5] = 0x90;
    // NOLINTEND(readability-magic-numbers)
}

}  // namespace unrealsdk::game

#endif
