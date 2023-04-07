#ifndef UNREALSDK_UNREAL_ALIGNMENT_H
#define UNREALSDK_UNREAL_ALIGNMENT_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

/**
 * @brief Helper to gets the alignment to use with the GMalloc functions, based on allocation size.
 *
 * @param len The amount of bytes requested to be allocated
 * @return The alignment.
 */
uint32_t get_malloc_alignment(size_t len);

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_ALIGNMENT_H */
