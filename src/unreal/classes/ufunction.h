#ifndef UNREAL_CLASSES_UFUNCTION_H
#define UNREAL_CLASSES_UFUNCTION_H

#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UFunction : public UStruct {
   public:
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,
    //             modernize-avoid-c-arrays,
    //             readability-identifier-naming)

#ifdef UE4
    uint32_t FunctionFlags;
    uint8_t NumParams;
    uint16_t ParamsSize;
    uint16_t ReturnValueOffset;

   private:
    uint16_t RPCId;
    uint16_t RPCResponseId;
    UProperty* FirstPropertyToInit;
    UFunction* EventGraphFunction;
    int32_t EventGraphCallOffset;
    void* Func;
#else
    uint32_t FunctionFlags;

   private:
    uint16_t iNative;
    uint16_t RepOffset;
    FName FriendlyName;
    uint8_t OperPrecedence;

   public:
    uint8_t NumParams;
    uint16_t ParamsSize;
    uint16_t ReturnValueOffset;

   private:
    uint8_t UnknownData00[0x6];
    void* Func;
#endif

    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,
    //           modernize-avoid-c-arrays,
    //           readability-identifier-naming)
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UFUNCTION_H */
