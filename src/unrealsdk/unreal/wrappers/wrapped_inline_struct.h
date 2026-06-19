#ifndef UNREALSDK_UNREAL_WRAPPERS_WRAPPED_INLINE_STRUCT_H
#define UNREALSDK_UNREAL_WRAPPERS_WRAPPED_INLINE_STRUCT_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fgbxinlinestruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

struct FGbxInlineStruct;

class WrappedInlineStruct : public WrappedStruct {
   public:
    // Since inline structs contain a shared pointer, copy our own reference to it to keep it alive
    FGbxInlineStruct inline_ref;

    /**
     * @brief Constructs a new wrapped inline struct.
     *
     * @param inline_ref Pointer to the base inline struct data.
     * @param other The other wrapped struct to copy/move from.
     */
    WrappedInlineStruct(const FGbxInlineStruct* inline_ref, const UnrealPointer<void>& parent);
    WrappedInlineStruct(const WrappedInlineStruct& other);
    WrappedInlineStruct(WrappedInlineStruct&& other) noexcept;

    /**
     * @brief Assigns to the struct.
     * @note Only allowed if of the same type.
     *
     * @param other The other wrapped struct to copy/move from.
     * @return A reference to this wrapped struct.
     */
    WrappedInlineStruct& operator=(const WrappedInlineStruct& other);
    WrappedInlineStruct& operator=(WrappedInlineStruct&& other) noexcept;

    /**
     * @brief Destroys the wrapped struct
     */
    ~WrappedInlineStruct();
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_WRAPPERS_WRAPPED_INLINE_STRUCT_H */
