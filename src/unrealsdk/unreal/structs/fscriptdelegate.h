#ifndef UNREALSDK_UNREAL_STRUCTS_FSCRIPTDELEGATE_H
#define UNREALSDK_UNREAL_STRUCTS_FSCRIPTDELEGATE_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fweakobjectptr.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UObject;

struct FScriptDelegate {
   private:
#ifdef UE3
    UObject* object = nullptr;
#else
    FWeakObjectPtr object;
#endif

   public:
    FName func_name;

    /**
     * @brief Get the object this delegate is bound to.
     *
     * @return The object.
     */
    [[nodiscard]] UObject* get_object(void) const;

    /**
     * @brief Sets the object this delegate is bound to.
     *
     * @param object The object.
     */
    void set_object(UObject* object);
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FSCRIPTDELEGATE_H */
