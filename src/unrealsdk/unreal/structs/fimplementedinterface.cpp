#include "pch.h"

#include "unreal/classes/properties/ustructproperty.h"
#include "unreal/structs/fimplementedinterface.h"

namespace unrealsdk::unreal {

size_t FImplementedInterface::get_pointer_offset() const {
#if UE4
    return this->isNative ? 0 : this->PointerOffset;
#else
    return this->VFTableProperty == nullptr ? 0 : this->VFTableProperty->Offset_Internal;
#endif
}

}  // namespace unrealsdk::unreal
