#ifndef UNREALSDK_UNREAL_CLASSES_UFUNCTION_H
#define UNREALSDK_UNREAL_CLASSES_UFUNCTION_H

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
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

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UFUNCTION_FIELDS(X) \
    X(uint32_t, FunctionFlags)        \
    X(uint8_t, NumParams)             \
    X(uint16_t, ParamsSize)           \
    X(uint16_t, ReturnValueOffset)

    UNREALSDK_DEFINE_FIELDS_HEADER(UFunction, UNREALSDK_UFUNCTION_FIELDS);

    // NOLINTBEGIN(readability-magic-numbers, readability-identifier-naming)

   private:
#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    uint32_t FunctionFlags_internal;
    uint8_t NumParams_internal;
    uint16_t ParamsSize_internal;
    uint16_t ReturnValueOffset_internal;
    uint16_t RPCId;
    uint16_t RPCResponseId;
    UProperty* FirstPropertyToInit;
    UFunction* EventGraphFunction;
    int32_t EventGraphCallOffset;
    void* Func;
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    uint32_t FunctionFlags_internal;
    uint16_t iNative;
    uint16_t RepOffset;
    FName FriendlyName;
    uint8_t OperPrecedence;
    uint8_t NumParams_internal;
    uint16_t ParamsSize_internal;
    uint16_t ReturnValueOffset_internal;
    uint8_t UnknownData00[0x6];
    void* Func;
#else
#error Unknown SDK flavour
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

template <>
struct ClassTraits<UFunction> {
    static inline const wchar_t* const NAME = L"Function";
};

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UFUNCTION_H */
