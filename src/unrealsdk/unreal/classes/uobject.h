#ifndef UNREALSDK_UNREAL_CLASSES_UOBJECT_H
#define UNREALSDK_UNREAL_CLASSES_UOBJECT_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

struct FImplementedInterface;
class UClass;

class UObject {
   public:
    // Prevent actually using as a class, must reinterpret_cast
    UObject() = delete;
    UObject(const UObject&) = delete;
    UObject(UObject&&) = delete;
    UObject& operator=(const UObject&) = delete;
    UObject& operator=(UObject&&) = delete;
    ~UObject() = delete;

    uintptr_t* vftable;

    // NOLINTBEGIN(readability-identifier-naming)

#if UE4
    int32_t ObjectFlags;
    int32_t InternalIndex;
    UClass* Class;
    FName Name;
    UObject* Outer;
#else
   private:
    void* HashNext;

   public:
    uint64_t ObjectFlags;

   private:
    void* HashOuterNext;
    void* StateFrame;
    class UObject* Linker;
    void* LinkerIndex;
    int ObjectInternalInteger;

   public:
    int InternalIndex;
    UObject* Outer;
    FName Name;
    UClass* Class;

   private:
    UObject* ObjectArchetype;

   public:
#endif

    // NOLINTEND(readability-identifier-naming)

    /**
     * @brief Calls a virtual function on this object.
     *
     * @tparam R The return type of the function. May be void.
     * @tparam Args The types of the function args. Should get picked up automatically.
     * @param index The index of the function in the vftable.
     * @param args The function's args.
     * @return The function's return value.
     */
    template <typename R, typename... Args>
    R call_virtual_function(size_t index, Args... args) const {
        return reinterpret_cast<R (*)(const UObject*, Args...)>(this->vftable[index])(this,
                                                                                      args...);
    }

    /**
     * @brief Gets a property on this object.
     *
     * @tparam T The type of the property.
     * @tparam V The value type of the property. Usually detected from property traits.
     * @param name The property's name to lookup.
     * @param prop The property to get.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @return The property's value.
     */
    template <typename T, typename V = typename PropTraits<T>::Value>
    [[nodiscard]] V get(const FName& name, size_t idx = 0) const;
    template <typename T, typename V = typename PropTraits<T>::Value>
    [[nodiscard]] V get(const T* prop, size_t idx = 0) const;

    /**
     * @brief Sets a property on this struct
     *
     * @tparam T The type of the property.
     * @param name The property's name to lookup.
     * @param prop The property to set.
     * @param idx The fixed array index to set the value at. Defaults to 0.
     * @param value The property's value.
     */
    template <typename T>
    void set(const FName& name, const typename PropTraits<T>::Value& value) {
        this->set<T>(name, 0, value);
    }
    template <typename T>
    void set(const T* prop, const typename PropTraits<T>::Value& value) {
        this->set<T>(prop, 0, value);
    }
    template <typename T>
    void set(const FName& name, size_t idx, const typename PropTraits<T>::Value& value);
    template <typename T>
    void set(const T* prop, size_t idx, const typename PropTraits<T>::Value& value);

    /**
     * @brief Get the object's full path name.
     *
     * @return The full path name.
     */
    [[nodiscard]] std::wstring get_path_name(void) const;

    /**
     * @brief Checks if this object is an instance of a class.
     * @note Does not check interfaces, only plain inheritance.
     *
     * @param cls The class to check.
     * @return True if this object is an instance of the given class.
     */
    [[nodiscard]] bool is_instance(const UClass* cls) const;

    /**
     * @brief Checks if this object implements an interface.
     *
     * @param iface The interface to check.
     * @param[out] impl_out If not null, gets set to the interface implementation for this object
     *                      (assuming it's found).
     * @return True if this object implements the given interface.
     */
    [[nodiscard]] bool is_implementation(const UClass* iface,
                                         FImplementedInterface** impl_out = nullptr) const;
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UOBJECT_H */
