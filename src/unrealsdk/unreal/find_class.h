#ifndef UNREALSDK_UNREAL_FIND_CLASS_H
#define UNREALSDK_UNREAL_FIND_CLASS_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/class_name.h"

namespace unrealsdk::unreal {

struct FName;
class UClass;

/**
 * @brief Finds a class by name.
 * @note If two classes (presumably from different packages) share a name, calling with their FName
 *       returns an undefined instance. Calling with a string performs a fully qualified lookup,
 *       which is less efficient, but properly handles this case.
 *
 * @tparam T The type to get the class object of.
 * @param name The class name.
 * @return The class object.
 */
template <typename T>
[[nodiscard]] UClass* find_class(void) {
    return find_class(cls_fname<T>());
}
[[nodiscard]] UClass* find_class(const FName& name);
[[nodiscard]] UClass* find_class(std::wstring_view name);

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_FIND_CLASS_H */
