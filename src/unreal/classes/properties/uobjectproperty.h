#ifndef UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H
#define UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H

#include "pch.h"

#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/prop_traits.h"

namespace unrealsdk::unreal {

class UClass;
class UObject;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UObjectProperty : public UProperty {
   private:
    friend PropTraits<UObjectProperty>;

    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* PropertyClass;

   public:
    /**
     * @brief Get the class of this property, which the values must be an instance of.
     *
     * @return This property's class.
     */
    [[nodiscard]] UClass* get_property_class(void) const;
};

template <>
struct PropTraits<UObjectProperty> {
    using Value = UObject*;
    static inline const wchar_t* const CLASS = L"ObjectProperty";

    static Value get(const UObjectProperty* prop, uintptr_t addr);
    static void set(const UObjectProperty* prop, uintptr_t addr, Value value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H */
