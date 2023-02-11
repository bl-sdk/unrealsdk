#ifndef UNREAL_WRAPPERS_WRAPPED_STRUCT_H
#define UNREAL_WRAPPERS_WRAPPED_STRUCT_H

#include "unreal/classes/ustruct.h"
#include "unreal/classes/ustruct_funcs.h"
#include "unreal/structs/fname.h"
#include "unreal/wrappers/prop_traits.h"

namespace unrealsdk::unreal {

class UStruct;

class WrappedStruct {
   public:
    const UStruct* type;
    void* base;

    /**
     * @brief Gets a property on this struct.
     *
     * @tparam T The type of the property.
     * @param name The property's name.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @return The property's new value.
     */
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(const FName& name, size_t idx = 0) const {
        return get_property<T>(this->type->find_and_validate<T>(name), idx,
                               reinterpret_cast<uintptr_t>(this->base));
    }

    /**
     * @brief Sets a property on this struct
     *
     * @tparam T The type of the property.
     * @param name The property's name.
     * @param idx The fixed array index to set the value at. Defaults to 0.
     * @param value The property's value.
     */
    template <typename T>
    void set(const FName& name, typename PropTraits<T>::Value value) {
        this->set<T>(name, 0, value);
    }
    template <typename T>
    void set(const FName& name, size_t idx, typename PropTraits<T>::Value value) {
        set_property<T>(this->type->find_and_validate<T>(name), idx,
                        reinterpret_cast<uintptr_t>(this->base), value);
    }
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_WRAPPED_STRUCT_H */
