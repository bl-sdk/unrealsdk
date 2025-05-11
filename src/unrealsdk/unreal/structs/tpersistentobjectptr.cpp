#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/structs/tpersistentobjectptr.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

namespace {

template <typename T>
const TPersistentObjectPtr<T>* get_addr_from(const UObject* obj,
                                             const UObjectProperty* prop,
                                             size_t idx) {
    if (std::cmp_greater_equal(idx, prop->ArrayDim())) {
        throw std::out_of_range("Property index out of range");
    }

    auto addr =
        reinterpret_cast<uintptr_t>(obj) + prop->Offset_Internal() + (idx * prop->ElementSize());
    return reinterpret_cast<TPersistentObjectPtr<T>*>(addr);
}
template <typename T>
const TPersistentObjectPtr<T>* get_addr_from(const WrappedStruct& wrapped_struct,
                                             const UObjectProperty* prop,
                                             size_t idx) {
    if (std::cmp_greater_equal(idx, prop->ArrayDim())) {
        throw std::out_of_range("Property index out of range");
    }

    auto addr = reinterpret_cast<uintptr_t>(wrapped_struct.base.get()) + prop->Offset_Internal()
                + (idx * prop->ElementSize());
    return reinterpret_cast<TPersistentObjectPtr<T>*>(addr);
}

template <typename T>
const TPersistentObjectPtr<T>* get_addr_from_array(const WrappedArray& array, size_t idx) {
    if (!array.type->Class()->inherits(find_class<USoftObjectProperty>())) {
        throw std::invalid_argument("WrappedArray property was of invalid type "
                                    + (std::string)array.type->Class()->Name());
    }
    if (std::cmp_greater_equal(idx, array.base->count)) {
        throw std::out_of_range("WrappedArray index out of range");
    }

    auto addr = reinterpret_cast<uintptr_t>(array.base->data) + (array.type->ElementSize() * idx);
    return reinterpret_cast<TPersistentObjectPtr<T>*>(addr);
}

}  // namespace

const FSoftObjectPath* FSoftObjectPath::get_from(const UObject* obj, FName name, size_t idx) {
    return get_from(obj, obj->Class()->find_prop_and_validate<USoftObjectProperty>(name), idx);
}
const FSoftObjectPath* FSoftObjectPath::get_from(const WrappedStruct& wrapped_struct,
                                                 FName name,
                                                 size_t idx) {
    return get_from(wrapped_struct,
                    wrapped_struct.type->find_prop_and_validate<USoftObjectProperty>(name), idx);
}
const FSoftObjectPath* FSoftObjectPath::get_from(const UObject* obj,
                                                 const USoftObjectProperty* prop,
                                                 size_t idx) {
    return &get_addr_from<FSoftObjectPath>(obj, prop, idx)->identifier;
}
const FSoftObjectPath* FSoftObjectPath::get_from(const WrappedStruct& wrapped_struct,
                                                 const USoftObjectProperty* prop,
                                                 size_t idx) {
    return &get_addr_from<FSoftObjectPath>(wrapped_struct, prop, idx)->identifier;
}

const FSoftObjectPath* FSoftObjectPath::get_from_array(const WrappedArray& array, size_t idx) {
    return &get_addr_from_array<FSoftObjectPath>(array, idx)->identifier;
}

const FLazyObjectPath* FLazyObjectPath::get_from(const UObject* obj, FName name, size_t idx) {
    return get_from(obj, obj->Class()->find_prop_and_validate<ULazyObjectProperty>(name), idx);
}
const FLazyObjectPath* FLazyObjectPath::get_from(const WrappedStruct& wrapped_struct,
                                                 FName name,
                                                 size_t idx) {
    return get_from(wrapped_struct,
                    wrapped_struct.type->find_prop_and_validate<ULazyObjectProperty>(name), idx);
}
const FLazyObjectPath* FLazyObjectPath::get_from(const UObject* obj,
                                                 const ULazyObjectProperty* prop,
                                                 size_t idx) {
    return &get_addr_from<FLazyObjectPath>(obj, prop, idx)->identifier;
}
const FLazyObjectPath* FLazyObjectPath::get_from(const WrappedStruct& wrapped_struct,
                                                 const ULazyObjectProperty* prop,
                                                 size_t idx) {
    return &get_addr_from<FLazyObjectPath>(wrapped_struct, prop, idx)->identifier;
}

const FLazyObjectPath* FLazyObjectPath::get_from_array(const WrappedArray& array, size_t idx) {
    return &get_addr_from_array<FLazyObjectPath>(array, idx)->identifier;
}

}  // namespace unrealsdk::unreal
