#include "pch.h"

#include "game/tps/tps.h"
#include "unreal/structs/fname.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

void TPSHook::fname_init(FName* name, const wchar_t* str, int32_t number) const {
    // NOLINTNEXTLINE(modernize-use-using)
    typedef void*(__thiscall * fname_init_func)(FName * name, const wchar_t* str,
                                                int32_t number, int32_t find_type,
                                                int32_t split_name, int32_t /*unknown*/);

    reinterpret_cast<fname_init_func>(this->fname_init_ptr)(name, str, number, 1, 1, 0);
}

}  // namespace unrealsdk::game

#endif
