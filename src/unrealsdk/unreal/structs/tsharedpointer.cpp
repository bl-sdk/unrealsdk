#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/tsharedpointer.h"
#include "unrealsdk/env.h"

namespace unrealsdk::unreal {

bool TReferenceController::remove_weak_ref(void) {
    if (--this->weak_ref_count > 1) {
        return false;
    }

    static auto idx =
        env::get_numeric<size_t>(env::TREFERENCE_CONTROLLER_DESTUCTOR_VF_INDEX,
                                 env::defaults::TREFERENCE_CONTROLLER_DESTUCTOR_VF_INDEX);
    reinterpret_cast<void (*)(TReferenceController* self, uint32_t should_delete)>(
        this->vftable[idx])(this, 1);
    return true;
}

bool TReferenceController::remove_strong_ref(void) {
    if (--this->ref_count > 1) {
        return false;
    }

    static auto idx =
        env::get_numeric<size_t>(env::TREFERENCE_CONTROLLER_DESTROY_OBJ_VF_INDEX,
                                 env::defaults::TREFERENCE_CONTROLLER_DESTROY_OBJ_VF_INDEX);
    reinterpret_cast<void (*)(TReferenceController* self)>(this->vftable[idx])(this);

    // The strong refs count as a single weak ref, when we run out of strong references, also remove
    // a weak reference.
    return this->remove_weak_ref();
}

}  // namespace unrealsdk::unreal
