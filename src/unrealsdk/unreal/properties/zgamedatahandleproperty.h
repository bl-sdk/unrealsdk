#ifndef UNREALSDK_UNREAL_PROPERTIES_ZGAMEDATAHANDLEPROPERTY_H
#define UNREALSDK_UNREAL_PROPERTIES_ZGAMEDATAHANDLEPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/properties/zproperty.h"
#include "unrealsdk/unreal/structs/fgamedatahandle.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

class ZGameDataHandleProperty : public ZProperty {
   public:
    ZGameDataHandleProperty() = delete;
    ZGameDataHandleProperty(const ZGameDataHandleProperty&) = delete;
    ZGameDataHandleProperty(ZGameDataHandleProperty&&) = delete;
    ZGameDataHandleProperty& operator=(const ZGameDataHandleProperty&) = delete;
    ZGameDataHandleProperty& operator=(ZGameDataHandleProperty&&) = delete;
    ~ZGameDataHandleProperty() = delete;

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_ZGAMEDATAHANDLEPROPERTY_FIELDS(X) X(uint32_t, TypeHandle)

    UNREALSDK_DEFINE_FIELDS_HEADER(ZGameDataHandleProperty,
                                   UNREALSDK_ZGAMEDATAHANDLEPROPERTY_FIELDS);
};

template <>
struct PropTraits<ZGameDataHandleProperty> : public AbstractPropTraits<ZGameDataHandleProperty> {
    using Value = FGameDataHandle;

    static Value get(const ZGameDataHandleProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const ZGameDataHandleProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const ZGameDataHandleProperty* prop, uintptr_t addr);
};

template <>
struct ClassTraits<ZGameDataHandleProperty> {
    static inline const wchar_t* const NAME = L"GameDataHandleProperty";
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_PROPERTIES_ZGAMEDATAHANDLEPROPERTY_H */
