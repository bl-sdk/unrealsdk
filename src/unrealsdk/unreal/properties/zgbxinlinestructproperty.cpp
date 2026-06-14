#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/properties/zgbxinlinestructproperty.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/structs/fgbxinlinestruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_inline_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(ZGbxInlineStructProperty,
                                    UNREALSDK_ZGBXINLINESTRUCTPROPERTY_FIELDS);

PropTraits<ZGbxInlineStructProperty>::Value PropTraits<ZGbxInlineStructProperty>::get(
    const ZGbxInlineStructProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& parent) {
    auto inline_ref = reinterpret_cast<FGbxInlineStruct*>(addr);
    if (inline_ref->ptr.obj == nullptr || inline_ref->ptr.controller == nullptr) {
        return std::nullopt;
    }
    return WrappedInlineStruct{inline_ref, parent};
}

void PropTraits<ZGbxInlineStructProperty>::set(const ZGbxInlineStructProperty* prop,
                                               uintptr_t addr,
                                               const Value& value) {
    if (value.has_value()) {
        auto prop_cls = prop->MetaStruct();
        if (!value->type->inherits(prop_cls)) {
            throw std::runtime_error("Inline struct type does not inherit from "
                                     + prop_cls->Name());
        }
    }

    // Whatever value we currently have, we're removing it
    auto inline_ref = reinterpret_cast<FGbxInlineStruct*>(addr);
    if (inline_ref->ptr.controller != nullptr) {
        inline_ref->ptr.controller->remove_strong_ref();
    }
    inline_ref->ptr = {.obj = nullptr, .controller = nullptr};
    inline_ref->flags = 0;

    if (value.has_value() && value->inline_ref.ptr.controller != nullptr) {
        // If the arg has a value, we're overwriting our value with its, add a reference
        value->inline_ref.ptr.controller->ref_count++;
        *inline_ref = value->inline_ref;
    }
    // If we didn't have a value, we'll keep the nulled struct
}

void PropTraits<ZGbxInlineStructProperty>::destroy(const ZGbxInlineStructProperty* /*prop*/,
                                                   uintptr_t addr) {
    auto inline_ref = reinterpret_cast<FGbxInlineStruct*>(addr);
    if (inline_ref->ptr.controller != nullptr) {
        inline_ref->ptr.controller->remove_strong_ref();
    }
    inline_ref->ptr = {.obj = nullptr, .controller = nullptr};
}

}  // namespace unrealsdk::unreal
