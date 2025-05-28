#include "unrealsdk/pch.h"

#include "unrealsdk/game/tps/tps.h"
#include "unrealsdk/memory.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

namespace {

const constinit Pattern<11> ARRAY_LIMIT_MESSAGE{
    "7C ??"           // jl BorderlandsPreSequel.exe+C3826
    "8B 8D ????????"  // mov ecx, [ebp-0000116C]
    "83 C0 9D"        // add eax, -63
};

}  // namespace

void TPSHook::hexedit_array_limit_message(void) const {
    auto array_limit_msg = ARRAY_LIMIT_MESSAGE.sigscan_nullable<uint8_t*>();
    if (array_limit_msg == nullptr) {
        LOG(ERROR, "Couldn't find array limit message signature");
    } else {
        LOG(MISC, "Array Limit Message: {:p}", reinterpret_cast<void*>(array_limit_msg));

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit_msg, 1);
        array_limit_msg[0] = 0xEB;
        // NOLINTEND(readability-magic-numbers)
    }
}

}  // namespace unrealsdk::game

#endif
