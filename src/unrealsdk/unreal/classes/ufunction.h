#ifndef UNREALSDK_UNREAL_CLASSES_UFUNCTION_H
#define UNREALSDK_UNREAL_CLASSES_UFUNCTION_H

#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

class UFunction : public UStruct {
   public:
    static constexpr auto FUNC_NATIVE = 0x400;

    UFunction() = delete;
    UFunction(const UFunction&) = delete;
    UFunction(UFunction&&) = delete;
    UFunction& operator=(const UFunction&) = delete;
    UFunction& operator=(UFunction&&) = delete;
    ~UFunction() = delete;

    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

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

   public:
    /**
     * @brief Finds the return param for this function (if it exists).
     *
     * @return The return param, or `nullptr` if none exists.
     */
    [[nodiscard]] UProperty* find_return_param(void) const;

    // NOLINTEND(readability-magic-numbers, readability-identifier-naming)
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UFUNCTION_H */
