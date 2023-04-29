#include "unrealsdk/pch.h"

#include "unrealsdk/game/tps/tps.h"
#include "unrealsdk/memory.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

namespace {

// This is actually the exact same sig as BL2 when we trim the leading any bytes, but the jump is
// encoded differently
const Pattern ARRAY_LIMIT_MESSAGE{"\x00\x00\x8B\x8D\x00\x00\x00\x00\x83\xC0\x9D",
                                  "\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF"};

}  // namespace

void TPSHook::hexedit_array_limit_message(void) const {
    auto array_limit_msg = sigscan<uint8_t*>(ARRAY_LIMIT_MESSAGE);
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
