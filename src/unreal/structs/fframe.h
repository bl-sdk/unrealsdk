#ifndef UNREAL_STRUCTS_FFRAME_H
#define UNREAL_STRUCTS_FFRAME_H

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

class UProperty;
class UFunction;
class UObject;

// NOLINTBEGIN(readability-identifier-naming)

struct FOutParmRec {
    UProperty* Property;
    void* PropAddr;
    FOutParmRec* NextOutParm;
};

struct FOutputDevice {
    void* VfTable;

#ifdef UE3
   private:
    uint32_t bAllowSuppression;

   public:
#endif

    uint32_t bSuppressEventTag;
    uint32_t bAutoEmitLineTerminator;
};

struct FFrame : public FOutputDevice {
    static constexpr const auto EXPR_TOKEN_END_FUNCTION_PARMS = 0x16;

    UFunction* Node;
    UObject* Object;
    uint8_t* Code;
    void* Locals;

#ifdef UE4
private:
    UProperty* LastProperty;
    void* LastPropertyAddress;
public:
#endif

    FFrame* PreviousFrame;
    FOutParmRec* OutParams;
};

// NOLINTEND(readability-identifier-naming)

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_FFRAME_H */
