#include "unrealsdk/pch.h"

#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/memory.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

namespace {

const constinit Pattern<28> SET_COMMAND_SIG{
    "75 ??"           // jne Borderlands2.exe+43019D
    "6A 01"           // push 01
    "8D 95 ????????"  // lea edx, [ebp-00000888]
    "68 ????????"     // push Borderlands2.exe+1265EE0
    "52"              // push edx
    "E8 ????????"     // call Borderlands2.exe+99350
    "83 C4 0C"        // add esp, 0C
    "85 C0"           // test eax, eax
    "74 ??"           // je Borderlands2.exe+4301C6
};

const constinit Pattern<9> ARRAY_LIMIT_SIG{
    "7E ??"        // jle Borderlands2.exe+C9ABB
    "B9 64000000"  // mov ecx, 00000064
    "3B F9"        // cmp edi, ecx
};

const constinit Pattern<15> ARRAY_LIMIT_MESSAGE{
    // Explicitly match the jump offset, since to overwrite this with an unconditional jump we need
    // to move where it takes place, hardcoding our own offset
    "0F 8C 7B000000"  // jl Borderlands2.exe+C9BF0
    "8B 8D ????????"  // mov ecx, [ebp-00001164]
    "83 C0 9D"        // add eax, -63
};

}  // namespace

void BL2Hook::hexedit_set_command(void) {
    auto set_command = SET_COMMAND_SIG.sigscan<uint8_t*>();
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
    auto array_limit = ARRAY_LIMIT_SIG.sigscan<uint8_t*>();
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
    auto array_limit_msg = ARRAY_LIMIT_MESSAGE.sigscan<uint8_t*>();
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
