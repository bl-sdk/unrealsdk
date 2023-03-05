#include "pch.h"

#include "game/tps/tps.h"
#include "memory.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::memory;

namespace unrealsdk::game {

void TPSHook::hexedit_array_limit_message(void) const {
    static const Pattern ARRAY_LIMIT_MESSAGE{"\x7C\x7B\x8B\x8D\x94\xEE\xFF\xFF",
                                             "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"};

    auto array_limit_msg = sigscan<uint8_t*>(ARRAY_LIMIT_MESSAGE);
    if (array_limit_msg == nullptr) {
        LOG(MISC, "Couldn't find array limit message signature, assuming already hex edited");
    } else {
        LOG(MISC, "Array Limit Message: 0x%p", array_limit_msg);

        // NOLINTBEGIN(readability-magic-numbers)
        unlock_range(array_limit_msg, 1);
        array_limit_msg[0] = 0x75;
        // NOLINTEND(readability-magic-numbers)
    }
}

}  // namespace unrealsdk::game

#endif
