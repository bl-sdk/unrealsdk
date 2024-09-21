#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

#ifdef UE3

UObject* FScriptDelegate::get_object(void) const {
    return this->object;
}

void FScriptDelegate::set_object(UObject* object) {
    this->object = object;
}

#else

UObject* FScriptDelegate::get_object(void) const {
    return unrealsdk::gobjects().get_weak_object(&this->object);
}

void FScriptDelegate::set_object(UObject* object) {
    unrealsdk::gobjects().set_weak_object(&this->object, object);
}
#endif

}  // namespace unrealsdk::unreal
