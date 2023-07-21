#ifndef UNREALSDK_UNREAL_CLASS_TRAITS_H
#define UNREALSDK_UNREAL_CLASS_TRAITS_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

class UObject;

/**
 * @brief Traits class describing the unreal object classes.
 *
 * @tparam T The class being described.
 */
template <typename T>
struct ClassTraits {
    /// The name of the described unreal class.
    /// Has to be a string literal as we can't initalize FNames immediately.
    static const wchar_t* const NAME;
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASS_TRAITS_H */
