#include "unrealsdk/pch.h"

#include "unrealsdk/game/tps/tps.h"
#include "unrealsdk/unreal/structs/fname.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"  // thiscall on non-class
#endif

void TPSHook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    // NOLINTNEXTLINE(modernize-use-using)
    typedef void*(__thiscall * fname_init_func)(FName * name, const wchar_t* str, int32_t number,
                                                int32_t find_type, int32_t split_name,
                                                int32_t /*unknown*/);

    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str, number, 1, 1, 0);
}

#if defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

}  // namespace unrealsdk::game

#endif
