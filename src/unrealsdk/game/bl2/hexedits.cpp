#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/memory.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

void BL2Hook::hexedit_set_command(void) {
    static const Pattern SET_COMMAND_SIG{"\x83\xC4\x0C\x85\xC0\x75\x1A\x6A\x01\x8D",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 5};

    auto set_command = sigscan<uint8_t*>(SET_COMMAND_SIG);
    if (set_command == nullptr) {
        LOG(MISC, "Couldn't find set command signature, assuming already hex edited");
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
    static const Pattern ARRAY_LIMIT_SIG{"\x7E\x05\xB9\x64\x00\x00\x00\x3B\xF9\x0F\x8D",
                                         "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto array_limit = sigscan<uint8_t*>(ARRAY_LIMIT_SIG);
    if (array_limit == nullptr) {
        LOG(MISC, "Couldn't find array limit signature, assuming already hex edited");
    } else {
        LOG(MISC, "Array Limit: {:p}", reinterpret_cast<void*>(array_limit));

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit, 1);
        array_limit[0] = 0xEB;
        // NOLINTEND(readability-magic-numbers)
    }
}

void BL2Hook::hexedit_array_limit_message(void) const {
    static const Pattern ARRAY_LIMIT_MESSAGE{
        "\x0F\x8C\x7B\x00\x00\x00\x8B\x8D\x9C\xEE\xFF\xFF\x83\xC0\x9D\x50",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto array_limit_msg = sigscan<uint8_t*>(ARRAY_LIMIT_MESSAGE);
    if (array_limit_msg == nullptr) {
        LOG(MISC, "Couldn't find array limit message signature, assuming already hex edited");
    } else {
        LOG(MISC, "Array Limit Message: {:p}", reinterpret_cast<void*>(array_limit_msg));

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit_msg + 1, 1);
        array_limit_msg[1] = 0x85;
        // NOLINTEND(readability-magic-numbers)
    }
}

}  // namespace unrealsdk::game

#endif
