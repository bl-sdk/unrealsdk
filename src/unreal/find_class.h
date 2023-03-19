#ifndef UNREAL_FIND_CLASS_H
#define UNREAL_FIND_CLASS_H

#include "pch.h"

namespace unrealsdk::unreal {

struct FName;
class UClass;

/**
 * @brief Finds a class by name.
 * @note If two classes (presumably from different packages) share a name, calling with their FName
 *       returns an undefined instance. Calling with a string performs a fully qualified lookup,
 *       which is less efficent, but properly handles this case.
 *
 * @param name The class name.
 * @return The class object.
 */
[[nodiscard]] UClass* find_class(const FName& name);
[[nodiscard]] UClass* find_class(const std::wstring& name);

}

#endif /* UNREAL_FIND_CLASS_H */
