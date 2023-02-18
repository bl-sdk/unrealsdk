#include "pch.h"

#include "unreal/classes/uclass.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/classes/ustruct_funcs.h"
#include "unreal/structs/fname.h"
#include "unreal/wrappers/bound_function.h"
#include "unreal/wrappers/prop_traits.h"

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

bool UObject::is_instance(const UClass* cls) const {
    for (const UStruct* obj_cls = this->Class; obj_cls != nullptr; obj_cls = obj_cls->SuperField) {
        if (obj_cls == cls) {
            return true;
        }
    }

    return false;
}

bool UObject::is_implementation(const UClass* iface, FImplementedInterface** impl_out) const {
    for (auto obj_cls = this->Class; obj_cls != nullptr;
         obj_cls = reinterpret_cast<UClass*>(obj_cls->SuperField)) {
        for (auto obj_iface : obj_cls->Interfaces) {
            if (obj_iface.Class == iface) {
                if (impl_out != nullptr) {
                    *impl_out = &obj_iface;
                }
                return true;
            }
        }
    }

    return false;
}

template <>
typename PropTraits<UFunction>::Value UObject::get<UFunction>(const UFunction* prop,
                                                              size_t idx) const {
    if (idx != 0) {
        throw std::out_of_range("Functions cannot have an array index!");
    }

    // All the other property getters are const, since obviously a getter shouldn't modify the
    // object. Since calling a function may do so however, bound functions need a mutable reference.
    // This means we need const casts. This is technically undefined behaviour, but since our code
    // only passes the reference around, and never modifies the object, we should be fine.

    // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
    auto mutable_func = const_cast<UFunction*>(prop);
    auto mutable_obj = const_cast<UObject*>(this);
    // NOLINTEND(cppcoreguidelines-pro-type-const-cast)

    return {mutable_func, mutable_obj};
}
template <>
typename PropTraits<UFunction>::Value UObject::get<UFunction>(const FName& name, size_t idx) const {
    return this->get<UFunction>(this->Class->find_and_validate<UFunction>(name), idx);
}

}  // namespace unrealsdk::unreal
