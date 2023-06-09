#ifndef UNREALSDK_UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H
#define UNREALSDK_UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"

namespace unrealsdk::unreal {

class UClass;
class UObject;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UObjectProperty : public UProperty {
   public:
    UObjectProperty() = delete;
    UObjectProperty(const UObjectProperty&) = delete;
    UObjectProperty(UObjectProperty&&) = delete;
    UObjectProperty& operator=(const UObjectProperty&) = delete;
    UObjectProperty& operator=(UObjectProperty&&) = delete;
    ~UObjectProperty() = delete;

    /**
     * @brief Get the class of this property, which the values must be an instance of.
     *
     * @return This property's class.
     */
    [[nodiscard]] UClass* get_property_class(void) const;

   private:
    friend PropTraits<UObjectProperty>;

    // NOLINTNEXTLINE(readability-identifier-naming)
    UClass* PropertyClass;
};

template <>
struct PropTraits<UObjectProperty> : public AbstractPropTraits<UObjectProperty> {
    using Value = UObject*;
    static inline const wchar_t* const CLASS = L"ObjectProperty";

    static Value get(const UObjectProperty* prop,
                     uintptr_t addr,
                     const std::shared_ptr<void>& parent);
    static void set(const UObjectProperty* prop, uintptr_t addr, const Value& value);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_PROPERTIES_UOBJECTPROPERTY_H */
