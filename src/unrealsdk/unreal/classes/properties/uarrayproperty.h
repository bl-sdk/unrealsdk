#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UARRAYPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UARRAYPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

class UArrayProperty : public UProperty {
   public:
    UArrayProperty() = delete;
    UArrayProperty(const UArrayProperty&) = delete;
    UArrayProperty(UArrayProperty&&) = delete;
    UArrayProperty& operator=(const UArrayProperty&) = delete;
    UArrayProperty& operator=(UArrayProperty&&) = delete;
    ~UArrayProperty() = delete;

    /**
     * @brief Get the inner property, which the array entries are instances of.
     *
     * @return This property's inner property.
     */
    [[nodiscard]] UProperty* get_inner(void) const;

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UProperty* Inner;
};

template <>
struct PropTraits<UArrayProperty> : public AbstractPropTraits<UArrayProperty> {
    using Value = WrappedArray;

    static Value get(const UArrayProperty* prop, uintptr_t addr, const UnrealPointer<void>& parent);
    static void set(const UArrayProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const UArrayProperty* prop, uintptr_t addr);
};

template <>
struct ClassTraits<UArrayProperty> {
    static inline const wchar_t* const NAME = L"ArrayProperty";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UARRAYPROPERTY_H */
