#ifndef UNREALSDK_UNREAL_FIND_CLASS_H
#define UNREALSDK_UNREAL_FIND_CLASS_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

struct FName;
class UClass;

/**
 * @brief Finds a class by name.
 * @note If two classes (presumably from different packages) share a name, calling with their FName
 *       returns an undefined instance. Calling with a string performs a fully qualified lookup,
 *       which is less efficient, but properly handles this case.
 *
 * @param name The class name.
 * @param name_size The size of the class name string.
 * @return The class object, or nullptr if unable to find.
 */
[[nodiscard]] UClass* find_class(const FName& name) noexcept;
[[nodiscard]] UClass* find_class(const std::wstring& name) noexcept;

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_FIND_CLASS_H */
