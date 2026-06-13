#ifndef UNREALSDK_UNREAL_STRUCTS_FGBXDEFPTR_H
#define UNREALSDK_UNREAL_STRUCTS_FGBXDEFPTR_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

/*
Current theory on this type:

For BL4 gearbox invented a custom .ncs "Nexus Config Store" file format, and tend to use it for
everything rather standard objects. In memory they're essentially just souped up datatables.

This type holds a *non-owning* reference into one these datatables.

When calling functions, you create a def without an instance pointer, and then inside the function
the game somehow resolves it. In some cases you can get away with just name (based on context?), in
some cases you need an explicit type. A name and type appear to uniquely identify a struct.

While we're less confident, def properties on objects, seem to work similarly - if there's no
instance pointer, then when the game next interacts with it, resolves it.

If you already have a resolved def, the game would *probably* treat it as read only, but we're going
to let you meddle with its properties.

Features we're going to ignore for now:
- Resolving properties ourselves - just let the game do it.
- Manually allocating properties - no one's investigated this yet.
- Changing the name/type of an already resolved property - you can reassign a new property on top of
  it. This means we can reuse this type as its own "wrapped def ptr" type.

ZGbxDefPtrPropertys have a Struct field. In lieu of other evidence, we're going to assume that's a
class restriction, same as an object property.
*/

struct FGbxDefPtr {
   public:
    FName name;
    UScriptStruct* type = nullptr;
    void* instance = nullptr;

    /**
     * @brief Constructs a new def pointer.
     *
     * @param name The name of the entry to resolve.
     * @param type The type of the entry to resolve.
     * @param other The other def pointer to copy from. Copies the reference.
     */
    FGbxDefPtr(void) = default;
    FGbxDefPtr(FName name, UScriptStruct* type = nullptr) : name(name), type(type) {}
    FGbxDefPtr(const FGbxDefPtr& other) = default;
    FGbxDefPtr(FGbxDefPtr&& other) noexcept = default;

    /**
     * @brief Copies the reference inside another def pointer into this one.
     *
     * @param other The other def pointer to copy from.
     * @return A reference to this pointer.
     */
    FGbxDefPtr& operator=(const FGbxDefPtr& other) = default;
    FGbxDefPtr& operator=(FGbxDefPtr&& other) noexcept = default;

    /**
     * @brief Destroys the def pointer
     */
    ~FGbxDefPtr(void) = default;

    /**
     * @brief Gets a property on the struct this points to, if it's been resolved.
     *
     * @tparam T The type of the property.
     * @param name The property's name to lookup.
     * @param prop The property to get.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @return The property's value.
     */
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(const FName& name, size_t idx = 0) const {
        if (this->instance == nullptr) {
            throw std::runtime_error("tried to get a property from an unresolved FGbxDefPtr");
        }
        return this->get<T>(this->type->find_prop_and_validate<T>(name), idx);
    }
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(const T* prop, size_t idx = 0) const {
        if (this->instance == nullptr) {
            throw std::runtime_error("tried to get a property from an unresolved FGbxDefPtr");
        }
        return get_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this->instance));
    }

    /**
     * @brief Sets a property on the struct this points to, if it's been resolved.
     *
     * @tparam T The type of the property.
     * @param name The property's name to lookup.
     * @param prop The property to set.
     * @param idx The fixed array index to set the value at. Defaults to 0.
     * @param value The property's new value.
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
    void set(const FName& name, size_t idx, const typename PropTraits<T>::Value& value) {
        if (this->instance == nullptr) {
            throw std::runtime_error("tried to get a property from an unresolved FGbxDefPtr");
        }
        return this->set<T>(this->type->find_prop_and_validate<T>(name), idx, value);
    }
    template <typename T>
    void set(const T* prop, size_t idx, const typename PropTraits<T>::Value& value) {
        if (this->instance == nullptr) {
            throw std::runtime_error("tried to get a property from an unresolved FGbxDefPtr");
        }
        set_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this->instance), value);
    }
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FGBXDEFPTR_H */
