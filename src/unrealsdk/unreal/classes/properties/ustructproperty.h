#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

class UScriptStruct;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UStructProperty : public UProperty {
   public:
    UStructProperty() = delete;
    UStructProperty(const UStructProperty&) = delete;
    UStructProperty(UStructProperty&&) = delete;
    UStructProperty& operator=(const UStructProperty&) = delete;
    UStructProperty& operator=(UStructProperty&&) = delete;
    ~UStructProperty() = delete;

    /**
     * @brief Get the struct type of this property, which values must be an instance of.
     *
     * @return This property's struct type.
     */
    [[nodiscard]] UScriptStruct* get_inner_struct(void) const;

   private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    UScriptStruct* Struct;
};

template <>
struct PropTraits<UStructProperty> : public AbstractPropTraits<UStructProperty> {
    using Value = WrappedStruct;
    static inline const wchar_t* const CLASS = L"StructProperty";

    static Value get(const UStructProperty* prop,
                     uintptr_t addr,
                     const UnrealPointer<void>& parent);
    static void set(const UStructProperty* prop, uintptr_t addr, const Value& value);
    static void destroy(const UStructProperty* prop, uintptr_t addr);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_USTRUCTPROPERTY_H */
