#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/memory.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

namespace {

const Pattern SET_COMMAND_SIG{
    "\x83\xC4\x0C\x85\xC0\x75\x1A\x6A\x01\x8D\x95\x00\x00\x00\x00\x68\x00\x00\x00\x00\x52\xE8\x00"
    "\x00\x00\x00\x83\xc4\x0C\x85\xC0\x74\x00",
    "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\x00\x00\x00\x00\xFF\xFF\x00"
    "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00",
    5};

const Pattern ARRAY_LIMIT_SIG{"\x00\x00\xB9\x64\x00\x00\x00\x3B\xF9\x0F\x8D\x00\x00\x00\x00",
                              "\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00"};

const Pattern ARRAY_LIMIT_MESSAGE{"\x00\x00\x00\x00\x00\x00\x8B\x8D\x00\x00\x00\x00\x83\xC0\x9D",
                                  "\x00\x00\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF"};

}  // namespace

void BL2Hook::hexedit_set_command(void) {
    auto set_command = sigscan<uint8_t*>(SET_COMMAND_SIG);
    if (set_command == nullptr) {
        LOG(ERROR, "Couldn't find set command signature");
    } else {
        LOG(MISC, "Set Command: {:p}", reinterpret_cast<void*>(set_command));

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(set_command, 2);
        set_command[0] = 0x90;
        set_command[1] = 0x90;
        // NOLINTEND(readability-magic-numbers)
    }
}

void BL2Hook::hexedit_array_limit(void) {
    auto array_limit = sigscan<uint8_t*>(ARRAY_LIMIT_SIG);
    if (array_limit == nullptr) {
        LOG(ERROR, "Couldn't find array limit signature");
    } else {
        LOG(MISC, "Array Limit: {:p}", reinterpret_cast<void*>(array_limit));

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit, 1);
        array_limit[0] = 0xEB;
        // NOLINTEND(readability-magic-numbers)
    }
}

void BL2Hook::hexedit_array_limit_message(void) const {
    auto array_limit_msg = sigscan<uint8_t*>(ARRAY_LIMIT_MESSAGE);
    if (array_limit_msg == nullptr) {
        LOG(ERROR, "Couldn't find array limit message signature");
    } else {
        LOG(MISC, "Array Limit Message: {:p}", reinterpret_cast<void*>(array_limit_msg));

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit_msg, 6);
        array_limit_msg[0] = 0xEB;
        array_limit_msg[1] = 0x7F;
        array_limit_msg[2] = 0x90;
        array_limit_msg[3] = 0x90;
        array_limit_msg[4] = 0x90;
        array_limit_msg[5] = 0x90;
        // NOLINTEND(readability-magic-numbers)
    }
}

}  // namespace unrealsdk::game

#endif
