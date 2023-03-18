#ifndef UNREAL_CLASS_NAME_H
#define UNREAL_CLASS_NAME_H

#include "unreal/classes/uclass.h"
#include "unreal/classes/uobject.h"
#include "unreal/prop_traits.h"
#include "unreal/structs/fname.h"

namespace unrealsdk::unreal {

/**
 * @brief Gets the unreal class name of the templated type.
 *
 * @tparam T The type to get the name of.
 * @return The class' fname.
 */
template <typename T>
[[nodiscard]] FName cls_fname(void) {
    static FName name{0, 0};
    static bool initalized = false;

    if (!initalized) {
        name = FName{PropTraits<T>::CLASS};
        initalized = true;
    }

    return name;
}

/**
 * @brief Validates that an object is of the expected type.
 * @note Uses an exact type match, not if it's an instance.
 *
 * @tparam T Type type the object is expected to be.
 * @param obj Pointer to the object.
 * @return The object cast to the expected type.
 */
template <typename T>
T* validate_type(UObject* obj) {
    static const auto EXPECTED_CLS_NAME = cls_fname<T>();
    auto cls_name = obj->Class->Name;
    if (cls_name != EXPECTED_CLS_NAME) {
        throw std::invalid_argument("Property was of invalid type " + (std::string)cls_name);
    }
    return reinterpret_cast<T*>(obj);
}

}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASS_NAME_H */
