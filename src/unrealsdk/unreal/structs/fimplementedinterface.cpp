#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/structs/fimplementedinterface.h"

namespace unrealsdk::unreal {

size_t FImplementedInterface::get_pointer_offset() const {
#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    return this->isNative ? 0 : this->PointerOffset;
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    return this->VFTableProperty == nullptr ? 0 : this->VFTableProperty->Offset_Internal();
#else
#error Unknown SDK flavour
#endif
}

}  // namespace unrealsdk::unreal
