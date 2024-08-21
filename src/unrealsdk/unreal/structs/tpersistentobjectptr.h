#ifndef UNREALSDK_UNREAL_STRUCTS_TPERSISTENTOBJECTPTR_H
#define UNREALSDK_UNREAL_STRUCTS_TPERSISTENTOBJECTPTR_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/fweakobjectptr.h"

namespace unrealsdk::unreal {

class ULazyObjectProperty;
class UObject;
class USoftObjectProperty;
class WrappedStruct;
class WrappedArray;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

template <typename Identifier>
struct TPersistentObjectPtr {
    FWeakObjectPtr weak_ptr;
    int32_t tag{};
    Identifier identifier;
};

struct FSoftObjectPath {
    FName asset_path_name;
    UnmanagedFString subpath;

    /**
     * @brief Get the soft object path from a property on an object/struct.
     *
     * @param obj The object to get the path from.
     * @param wrapped_struct The struct to get from.
     * @param name The name of the property to get the path from.
     * @param prop The property to get the path from.
     * @param idx The fixed array index to get from.
     * @return A pointer to the soft object path.
     */
    static const FSoftObjectPath* get_from(const UObject* obj, FName name, size_t idx = 0);
    static const FSoftObjectPath* get_from(const WrappedStruct& wrapped_struct,
                                           FName name,
                                           size_t idx = 0);
    static const FSoftObjectPath* get_from(const UObject* obj,
                                           const USoftObjectProperty* prop,
                                           size_t idx = 0);
    static const FSoftObjectPath* get_from(const WrappedStruct& wrapped_struct,
                                           const USoftObjectProperty* prop,
                                           size_t idx = 0);

    /**
     * @brief Get the soft object path from an array.
     *
     * @param array The array to get the path from.
     * @param idx The index in the array to get from.
     * @return A pointer to the soft object path.
     */
    static const FSoftObjectPath* get_from_array(const WrappedArray& array, size_t idx);
};

struct FLazyObjectPath {
    int32_t guid_a;
    int32_t guid_b;
    int32_t guid_c;
    int32_t guid_d;

    /**
     * @brief Get the lazy object path from a property on an object/struct.
     *
     * @param obj The object to get the path from.
     * @param wrapped_struct The struct to get from.
     * @param name The name of the property to get the path from.
     * @param prop The property to get the path from.
     * @param idx The fixed array index to get from.
     * @return A pointer to the lazy object path.
     */
    static const FLazyObjectPath* get_from(const UObject* obj, FName name, size_t idx = 0);
    static const FLazyObjectPath* get_from(const WrappedStruct& wrapped_struct,
                                           FName name,
                                           size_t idx = 0);
    static const FLazyObjectPath* get_from(const UObject* obj,
                                           const ULazyObjectProperty* prop,
                                           size_t idx = 0);
    static const FLazyObjectPath* get_from(const WrappedStruct& wrapped_struct,
                                           const ULazyObjectProperty* prop,
                                           size_t idx = 0);

    /**
     * @brief Get the lazy object path from an array.
     *
     * @param array The array to get the path from.
     * @param idx The index in the array to get from.
     * @return A pointer to the lazy object path.
     */
    static const FLazyObjectPath* get_from_array(const WrappedArray& array, size_t idx);
};

struct FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath> {};
struct FLazyObjectPtr : public TPersistentObjectPtr<FLazyObjectPath> {};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_TPERSISTENTOBJECTPTR_H */
