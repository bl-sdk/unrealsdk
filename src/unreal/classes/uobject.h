#ifndef UNREAL_CLASSES_UOBJECT_H
#define UNREAL_CLASSES_UOBJECT_H

#include "unreal/structs/fname.h"

namespace unrealsdk::unreal {

class UClass;

class UObject {
   public:
    // Prevent actually using as a class, must reinterpret_cast
    UObject() = delete;
    UObject(const UObject&) = delete;
    UObject(UObject&&) = delete;
    UObject& operator=(const UObject&) = delete;
    UObject& operator=(UObject&&) = delete;
    ~UObject() = delete;

    void* vftable;

    // NOLINTBEGIN(readability-identifier-naming)

#if UE4
    int32_t ObjectFlags;
    int32_t InternalIndex;
    UClass* Class;
    FName Name;
    UObject* Outer;
#else
    void* HashNext;
    uint64_t ObjectFlags;
    void* HashOuterNext;
    void* StateFrame;
    class UObject* Linker;
    void* LinkerIndex;
    int ObjectInternalInteger;
    int InternalIndex;
    class UObject* Outer;
    struct FName Name;
    class UClass* Class;
    class UObject* ObjectArchetype;
#endif

    // NOLINTEND(readability-identifier-naming)
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UOBJECT_H */