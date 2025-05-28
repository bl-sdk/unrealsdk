#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl1/bl1.h"
#include "unrealsdk/memory.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

namespace {

const constinit Pattern<32> SET_COMMAND_SIG{
    "85 C0"           // test eax, eax
    "{?? ??}"         // jne 0087EC57
    "8D 4C 24 ??"     // lea ecx, [esp+18]
    "68 ????????"     // push 01B167C0
    "51"              // push ecx
    "E8 ????????"     // call 005C2FD0
    "83 C4 08"        // add esp, 08
    "85 C0"           // test eax, eax
    "74 ??"           // je 0087EC80
    "39 9E ????????"  // cmp [esi+000003E0], ebx
};

const constinit Pattern<32> ARRAY_LIMIT_SIG{
    "6A 64"            // push 64
    "50"               // push eax
    "46"               // inc esi
    "{?? ????????}"    // call MIN              <---
    "83 C4 08"         // add esp, 08
    "3B F0"            // cmp esi, eax
    "0F8C ????????"    // jl 005E8F03
    "8B 7F ??"         // mov edi, [edi+04]
    "83 FF 64"         // cmp edi, 64
    "{???? ????????}"  // jl DONT_PRINT_MSG     <---
};
const constexpr auto ARRAY_LIMIT_MESSAGE_OFFSET_FROM_MIN = 5 + 3 + 2 + 6 + 3 + 3;
const constexpr auto ARRAY_LIMIT_UNLOCK_SIZE = ARRAY_LIMIT_MESSAGE_OFFSET_FROM_MIN + 2;

}  // namespace

void BL1Hook::hexedit_set_command(void) {
    auto* set_command_msg = SET_COMMAND_SIG.sigscan_nullable<uint8_t*>();

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
    auto array_limit = ARRAY_LIMIT_SIG.sigscan_nullable<uint8_t*>();
    if (array_limit == nullptr) {
        LOG(ERROR, "Couldn't find array limit signature");
        return;
    }

    LOG(MISC, "Array Limit: {:p}", reinterpret_cast<void*>(array_limit));
    unlock_range(array_limit, ARRAY_LIMIT_UNLOCK_SIZE);

    // To patch the array limit, we simply NOP out the call to min, so it always uses the full array
    // size. Luckily, this means we don't need to do any stack work.

    // NOLINTBEGIN(readability-magic-numbers)
    array_limit[0] = 0x90;
    array_limit[1] = 0x90;
    array_limit[2] = 0x90;
    array_limit[3] = 0x90;
    array_limit[4] = 0x90;
    // NOLINTEND(readability-magic-numbers)

    // Then for the message, we're patching the JL with a NOP then a JMP w/ 4 byte offset
    // Doing it this way means we can reuse the offset from the original JL

    auto array_limit_msg = array_limit + ARRAY_LIMIT_MESSAGE_OFFSET_FROM_MIN;
    // NOLINTBEGIN(readability-magic-numbers)
    array_limit_msg[0] = 0x90;
    array_limit_msg[1] = 0xE9;
    // NOLINTEND(readability-magic-numbers)
}

}  // namespace unrealsdk::game

#endif
