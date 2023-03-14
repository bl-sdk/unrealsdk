#include "pch.h"

#include "env.h"
#include "unreal/alignment.h"

namespace unrealsdk::unreal {

uint32_t get_malloc_alignment(size_t len) {
    static auto override = env::get_numeric<uint32_t>(env::ALLOC_ALIGNMENT);
    if (override != 0) {
        return override;
    }

    // Not all UE versions support alignment 0 = auto, so reimplement it ourselves
    static const constexpr auto LARGE_ALIGNMENT_THRESHOLD = 16;
    static const constexpr auto LARGE_ALIGNMENT = 16;
    static const constexpr auto SMALL_ALIGNMENT = 8;

    return len >= LARGE_ALIGNMENT_THRESHOLD ? LARGE_ALIGNMENT : SMALL_ALIGNMENT;
}

}  // namespace unrealsdk::unreal
