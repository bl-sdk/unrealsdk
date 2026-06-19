#ifndef UNREALSDK_UNREAL_PROPERTIES_ZINLINESTRUCTPROPERTY_H
#define UNREALSDK_UNREAL_PROPERTIES_ZINLINESTRUCTPROPERTY_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/properties/zstructproperty.h"
#include "unrealsdk/unreal/wrappers/wrapped_inline_struct.h"

namespace unrealsdk::unreal {

class UScriptStruct;

class ZGbxInlineStructProperty : public ZStructProperty {
   public:
    ZGbxInlineStructProperty() = delete;
    ZGbxInlineStructProperty(const ZGbxInlineStructProperty&) = delete;
    ZGbxInlineStructProperty(ZGbxInlineStructProperty&&) = delete;
    ZGbxInlineStructProperty& operator=(const ZGbxInlineStructProperty&) = delete;
    ZGbxInlineStructProperty& operator=(ZGbxInlineStructProperty&&) = delete;
    ~ZGbxInlineStructProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_ZGBXINLINESTRUCTPROPERTY_FIELDS(X) X(UScriptStruct*, MetaStruct)

    UNREALSDK_DEFINE_FIELDS_HEADER(ZGbxInlineStructProperty,
                                   UNREALSDK_ZGBXINLINESTRUCTPROPERTY_FIELDS);
};

template <>
struct PropTraits<ZGbxInlineStructProperty> : public AbstractPropTraits<ZGbxInlineStructProperty> {
    using Value = std::optional<WrappedInlineStruct>;

    static Value get(const ZGbxInlineStructProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const ZGbxInlineStructProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const ZGbxInlineStructProperty* prop, uintptr_t addr);
};

template <>
struct ClassTraits<ZGbxInlineStructProperty> {
    static inline const wchar_t* const NAME = L"GbxInlineStructProperty";
};
}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_PROPERTIES_ZINLINESTRUCTPROPERTY_H */
