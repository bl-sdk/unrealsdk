#ifndef UNREALSDK_UNREAL_STRUCTS_FFRAME_H
#define UNREALSDK_UNREAL_STRUCTS_FFRAME_H

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

class UProperty;
class UFunction;
class UObject;

class WrappedStruct;

// NOLINTBEGIN(readability-identifier-naming)

struct FOutParamRec {
    UProperty* Property;
    void* PropAddr;
    FOutParamRec* NextOutParam;
};

struct FOutputDevice {
    void* VfTable;

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
   private:
    uint32_t bAllowSuppression;

   public:
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    // Intentionally empty
#else
#error Unknown SDK flavour
#endif

    uint32_t bSuppressEventTag;
    uint32_t bAutoEmitLineTerminator;
};

struct FFrame : public FOutputDevice {
    static constexpr const auto EXPR_TOKEN_END_FUNCTION_PARAMS = 0x16;

    UFunction* Node;
    UObject* Object;
    uint8_t* Code;
    void* Locals;

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
   private:
    UProperty* LastProperty;
    void* LastPropertyAddress;

   public:
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    // Intentionally empty
#else
#error Unknown SDK flavour
#endif

    FFrame* PreviousFrame;
    FOutParamRec* OutParams;

    /**
     * @brief Extracts the current function args, assuming stopped during the `CallFunction` hook.
     * @note Steps `Code` to do so, leaving it pointing at the end of function params token.
     *
     * @param args The args struct to write to. Assumes type is already pointing to the function.
     * @return The original position of `Code`.
     */
    uint8_t* extract_current_args(WrappedStruct& args);
};

// NOLINTEND(readability-identifier-naming)

#if defined(__clang__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FFRAME_H */
