#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/properties/zgamedatahandleproperty.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fgamedatahandle.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(ZGameDataHandleProperty,
                                    UNREALSDK_ZGAMEDATAHANDLEPROPERTY_FIELDS);

PropTraits<ZGameDataHandleProperty>::Value PropTraits<ZGameDataHandleProperty>::get(
    const ZGameDataHandleProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    return *reinterpret_cast<FGameDataHandle*>(addr);
}

void PropTraits<ZGameDataHandleProperty>::set(const ZGameDataHandleProperty* prop,
                                              uintptr_t addr,
                                              const Value& value) {
    // Current assumption is the types must match
    auto prop_type = prop->TypeHandle();
    if (value.type_handle != prop_type) {
        throw std::runtime_error(
            std::format("FGameDataHandle type mismatch, expected {}", prop_type));
    }

    *reinterpret_cast<FGameDataHandle*>(addr) = value;
}

void PropTraits<ZGameDataHandleProperty>::destroy(const ZGameDataHandleProperty* /*prop*/,
                                                  uintptr_t /*addr*/) {
    // Current assumption is the handles are non-owning references, so we don't need to free
    // anything here
}

}  // namespace unrealsdk::unreal
