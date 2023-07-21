#ifndef UNREALSDK_UNREAL_CLASSES_UBLUEPRINTGENERATEDCLASS_H
#define UNREALSDK_UNREAL_CLASSES_UBLUEPRINTGENERATEDCLASS_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uclass.h"

namespace unrealsdk::unreal {

class UBlueprintGeneratedClass : public UClass {
   public:
    UBlueprintGeneratedClass() = delete;
    UBlueprintGeneratedClass(const UBlueprintGeneratedClass&) = delete;
    UBlueprintGeneratedClass(UBlueprintGeneratedClass&&) = delete;
    UBlueprintGeneratedClass& operator=(const UBlueprintGeneratedClass&) = delete;
    UBlueprintGeneratedClass& operator=(UBlueprintGeneratedClass&&) = delete;
    ~UBlueprintGeneratedClass() = delete;
};

template <>
struct ClassTraits<UBlueprintGeneratedClass> {
    static inline const wchar_t* const NAME = L"BlueprintGeneratedClass";
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UBLUEPRINTGENERATEDCLASS_H */
