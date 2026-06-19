#ifndef UNREALSDK_UNREAL_STRUCTS_FINLINESTRUCT_H
#define UNREALSDK_UNREAL_STRUCTS_FINLINESTRUCT_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fgbxdef.h"
#include "unrealsdk/unreal/structs/tsharedpointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

class UScriptStruct;

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

struct FGbxInlineStruct {
    TSharedPointer<FGbxDef> ptr{};
    uint64_t flags{};
};

namespace internal {

// Layout used when we allocate our own inline struct
struct FGbxInlineStructAllocation {
    static constexpr auto EXTRA_ALLOC_SIZE = 0x18;

    TReferenceController controller{.vftable = nullptr, .ref_count = 1, .weak_ref_count = 1};
    // This layout is important, this field is explicitly used in the vftables we're copying
    uint32_t offset_to_data = EXTRA_ALLOC_SIZE;
    uint32_t unknown = 0;
    FGbxDef def{};
    // Note the real def we're allocating is bigger than our FGbxDef type, extra fields go here.
};

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2
static_assert(offsetof(FGbxInlineStructAllocation, def)
                  == FGbxInlineStructAllocation::EXTRA_ALLOC_SIZE,
              "inline struct allocation layout was unexpectedly large");
#endif

}  // namespace internal

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FINLINESTRUCT_H */
