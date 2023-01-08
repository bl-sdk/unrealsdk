#include "pch.h"

#include "unreal/classes/uclass.h"
#include "unreal/classes/uobject.h"

namespace unrealsdk::unreal {

// TODO: replace with call directly to `UObject::[Get]PathName()`
/**
 * @brief Recursive helper to generate full object path name.
 *
 * @tparam T The type of string to get the name in.
 * @param obj The object to get the path name of.
 * @param stream The stream to push the object name onto.
 */
template <typename T>
static void iter_path_name(const UObject* obj, std::basic_stringstream<T>& stream) {
    if (obj->Outer != nullptr) {
        iter_path_name(obj->Outer, stream);

        static const FName PACKAGE_NAME = L"Package"_fn;
        if (obj->Outer->Class->Name != PACKAGE_NAME
            && obj->Outer->Outer->Class->Name == PACKAGE_NAME) {
            if constexpr (std::is_same_v<T, char>) {
                stream << ':';
            } else {
                stream << L':';
            }
        } else {
            if constexpr (std::is_same_v<T, char>) {
                stream << '.';
            } else {
                stream << L'.';
            }
        }
    }
    stream << obj->Name;
}

template <>
std::string UObject::get_path_name(void) const {
    std::stringstream stream;
    iter_path_name(this, stream);
    return stream.str();
}

template <>
std::wstring UObject::get_path_name(void) const {
    std::wstringstream stream;
    iter_path_name(this, stream);
    return stream.str();
}

}  // namespace unrealsdk::unreal
