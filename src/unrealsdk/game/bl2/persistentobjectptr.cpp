#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl2/bl2.h"
#include "unrealsdk/version_error.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::game {

void BL2Hook::fsoftobjectptr_assign(unreal::FSoftObjectPtr* /* ptr */,
                                    const unreal::UObject* /* obj */) const {
    throw_version_error("Soft object pointers are not implemented in UE3");
}

void BL2Hook::flazyobjectptr_assign(unreal::FLazyObjectPtr* /* ptr */,
                                    const unreal::UObject* /* obj */) const {
    throw_version_error("Lazy object pointers are not implemented in UE3");
}

}  // namespace unrealsdk::game

#endif
