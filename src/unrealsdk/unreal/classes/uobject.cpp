#include <sstream>
#include "unrealsdk/pch.h"


#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/ustruct_funcs.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

#ifdef UNREALSDK_INTERNAL_PATH_NAME

/**
 * @brief Recursive helper to generate full object path name.
 *
 * @tparam T The type of string to get the name in.
 * @param obj The object to get the path name of.
 * @param stream The stream to push the object name onto.
 */
static void iter_path_name(const UObject* obj, std::wstringstream& stream) {
    if (obj->Outer != nullptr) {
        iter_path_name(obj->Outer, stream);

        static const FName PACKAGE_NAME = L"Package"_fn;
        if (obj->Outer->Class->Name != PACKAGE_NAME
            && obj->Outer->Outer->Class->Name == PACKAGE_NAME) {
            stream << L':';
        } else {
            stream << L'.';
        }
    }
    stream << obj->Name;
}

std::wstring UObject::get_path_name(void) const {
    std::wstringstream stream;
    iter_path_name(this, stream);
    return stream.str();
}

#else

std::wstring UObject::get_path_name(void) const {
    return unrealsdk::uobject_path_name(this);
}

#endif

bool UObject::is_instance(const UClass* cls) const {
    return this->Class->inherits(cls);
}

bool UObject::is_implementation(const UClass* iface, FImplementedInterface** impl_out) const {
    return this->Class->implements(iface, impl_out);
}

template <>
BoundFunction UObject::get<UFunction, BoundFunction>(const UFunction* prop, size_t idx) const {
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
BoundFunction UObject::get<UFunction, BoundFunction>(const FName& name, size_t idx) const {
    return this->get<UFunction, BoundFunction>(this->Class->find_func_and_validate(name), idx);
}

}  // namespace unrealsdk::unreal
