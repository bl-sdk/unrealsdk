#ifndef UNREALSDK_UNREAL_STRUCTS_FINLINESTRUCT_H
#define UNREALSDK_UNREAL_STRUCTS_FINLINESTRUCT_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/tsharedpointer.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

class UScriptStruct;

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

/*
So this type is just a plain lie from gearbox - structs are inline normally, this type is explictly
out-of-line. It mostly seems like they're just trying to avoid using UObjects, but still want
something that behaves the exact same. They seem strictly worse, I suspect they're LLM slop.

The main thing to note with this type is that it explictly allows for subclasses of the MetaStruct
on the property. We have to use a virtual function to get the real type back out. All of the structs
that may be assigned must have a vftable (it's referenced by TReferenceController::DestoryObject()),
though it's accounted for in the property offsets so can mostly be ignored.
*/

struct FGbxInlineStruct {
    TSharedPointer<void> ptr{};
    uint64_t flags{};

    /**
     * @brief Gets the type of the contained struct.
     * @note Returns null if this is a null pointer.
     *
     * @return The contained struct type.
     */
    [[nodiscard]] UScriptStruct* get_type(void) const;
};

namespace internal {

// Layout used when we allocate our own inline struct
struct FGbxInlineStructAllocation {
    static constexpr auto EXTRA_ALLOC_SIZE = 0x18;

    TReferenceController controller{.vftable = nullptr, .ref_count = 1, .weak_ref_count = 1};
    // This layout is important, this field is explictly used in the vftables we're copying
    uint32_t offset_to_data = EXTRA_ALLOC_SIZE;
    uint32_t unknown = 0;
    uintptr_t* data_vftable = nullptr;
    // remaining struct data goes here
};

static_assert(offsetof(FGbxInlineStructAllocation, data_vftable)
                  == FGbxInlineStructAllocation::EXTRA_ALLOC_SIZE,
              "inline struct allocation layout was unexpectedly large");

}  // namespace internal

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FINLINESTRUCT_H */
