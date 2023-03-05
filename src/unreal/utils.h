#ifndef UNREAL_UTILS_H
#define UNREAL_UTILS_H

#include "pch.h"

namespace unrealsdk::unreal {

/**
 * @brief Helper to gets the alignment to use with the GMalloc functions, based on allocation size.
 *
 * @param len The amount of bytes requested to be allocated
 * @return The alignment.
 */
uint32_t get_malloc_alignment(size_t len);

}  // namespace unrealsdk::unreal

#endif /* UNREAL_UTILS_H */
