#ifndef UNREALSDK_UNREAL_CLASSES_UOBJECT_H
#define UNREALSDK_UNREAL_CLASSES_UOBJECT_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_traits.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

struct FImplementedInterface;
class UClass;
class UProperty;

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

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK
    using object_flags_type = uint32_t;
#elif UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
    using object_flags_type = uint64_t;
#else
#error Unknown SDK flavour
#endif

    // These fields become member functions, returning a reference into the object.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREALSDK_UOBJECT_FIELDS(X)   \
    X(object_flags_type, ObjectFlags) \
    X(int32_t, InternalIndex)         \
    X(UClass*, Class)                 \
    X(FName, Name)                    \
    X(UObject*, Outer)

    UNREALSDK_DEFINE_FIELDS_HEADER(UObject, UNREALSDK_UOBJECT_FIELDS);

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
        if constexpr (sizeof(void*) == sizeof(uint32_t)) {
#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"  // thiscall on non-class
#endif
            // NOLINTNEXTLINE(modernize-use-using) - need a typedef for the __thiscall
            typedef R(__thiscall * func_ptr)(const UObject*, Args...);
#if defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif
            return reinterpret_cast<func_ptr>(this->vftable[index])(this, args...);
        } else {
            using func_ptr = R (*)(const UObject*, Args...);
            return reinterpret_cast<func_ptr>(this->vftable[index])(this, args...);
        }
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
                                         FImplementedInterface* impl_out = nullptr) const;

    /**
     * @brief Notifies the engine that we've made an external change to a property.
     *
     * @param name The name of the property which was changed.
     * @param prop The property which was changed.
     * @param chain The chain of properties to follow, if the change was within a struct.
     */
    void post_edit_change_property(const FName& name) const;
    void post_edit_change_property(UProperty* prop) const;
    void post_edit_change_chain_property(UProperty* prop,
                                         const std::vector<UProperty*>& chain) const;
    void post_edit_change_chain_property(UProperty* prop,
                                         std::initializer_list<UProperty*> chain) const;
};

template <>
struct ClassTraits<UObject> {
    static inline const wchar_t* const NAME = L"Object";
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_CLASSES_UOBJECT_H */
