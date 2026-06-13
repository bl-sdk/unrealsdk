#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/properties/zgbxdefptrproperty.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(ZGbxDefPtrProperty, UNREALSDK_ZGBXDEFPTRPROPERTY_FIELDS);

PropTraits<ZGbxDefPtrProperty>::Value PropTraits<ZGbxDefPtrProperty>::get(
    const ZGbxDefPtrProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    return *reinterpret_cast<FGbxDefPtr*>(addr);
}

void PropTraits<ZGbxDefPtrProperty>::set(const ZGbxDefPtrProperty* prop,
                                         uintptr_t addr,
                                         const Value& value) {
    // Current assumption is, if you have a type, it must inherit from the struct in the property
    if (value.type != nullptr) {
        auto prop_cls = prop->Struct();
        if (!value.type->inherits(prop_cls)) {
            throw std::runtime_error("Ref type does not inherit from " + prop_cls->Name());
        }
    }

    *reinterpret_cast<FGbxDefPtr*>(addr) = value;
}

void PropTraits<ZGbxDefPtrProperty>::destroy(const ZGbxDefPtrProperty* /*prop*/,
                                             uintptr_t /*addr*/) {
    // Current assumption is the pointers are non-owning references, so we don't need to free
    // anything here
}

}  // namespace unrealsdk::unreal
