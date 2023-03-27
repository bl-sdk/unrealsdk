#ifndef UNREAL_WRAPPERS_WRAPPED_STRUCT_H
#define UNREAL_WRAPPERS_WRAPPED_STRUCT_H

#include "pch.h"

#include "unreal/classes/ustruct.h"
#include "unreal/classes/ustruct_funcs.h"
#include "unreal/prop_traits.h"
#include "unreal/structs/fname.h"

namespace unrealsdk::unreal {

class UStruct;

class ReadOnlyWrappedStruct {
   public:
    const UStruct* type;
    std::shared_ptr<void> base;

    /**
     * @brief Constructs a new wrapped struct.
     * @note Copies parent ownership if given, otherwise does not manage the given base address.
     *
     * @param type The type of the struct.
     * @param base The base address of the struct.
     * @param parent The parent pointer this struct was retrieved from, used to copy ownership.
     * @param other The other wrapped struct to move from. Only allowed if of the same type.
     */
    ReadOnlyWrappedStruct(const UStruct* type,
                          void* base,
                          const std::shared_ptr<void>& parent = {nullptr});
    ReadOnlyWrappedStruct(ReadOnlyWrappedStruct&& other) noexcept;

   protected:
    ReadOnlyWrappedStruct(const UStruct* type, const std::shared_ptr<void>&& base);

   public:
    /**
     * @brief Moves another struct into this one.
     *
     * @param other The other wrapped struct to move from
     * @return A reference to this wrapped struct.
     */
    ReadOnlyWrappedStruct& operator=(ReadOnlyWrappedStruct&& other) noexcept;

    /**
     * @brief Destroys the wrapped struct
     */
    ~ReadOnlyWrappedStruct() = default;

    /**
     * @brief Gets a property on this struct.
     *
     * @tparam T The type of the property.
     * @param name The property's name to lookup.
     * @param prop The property to get.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @return The property's value.
     */
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(const FName& name, size_t idx = 0) const {
        return this->get<T>(this->type->find_prop_and_validate<T>(name), idx);
    }
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(const T* prop, size_t idx = 0) const {
        return get_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this->base.get()),
                               this->base);
    }

    // As a read-only reference, we really shouldn't be copying anything, so explictly delete copy
    // construct/assignment
    ReadOnlyWrappedStruct(const ReadOnlyWrappedStruct& other) = delete;
    ReadOnlyWrappedStruct& operator=(const ReadOnlyWrappedStruct& other) = delete;
};

class WrappedStruct : public ReadOnlyWrappedStruct {
   public:
    /**
     * @brief Constructs a new wrapped struct.
     * @note If just the type is given, allocates new memory (which we manage) for the properties.
     * @note If a parent is given, copies it's ownership.
     * @note Otherwise, does not manage the given base address.
     *
     * @param type The type of the struct.
     * @param base The base address of the struct.
     * @param parent The parent pointer this struct was retrieved from, used to copy ownership.
     * @param other The other wrapped struct to copy/move from. Only allowed if of the same type.
     */
    WrappedStruct(const UStruct* type);
    WrappedStruct(const UStruct* type, void* base, const std::shared_ptr<void>& parent = {nullptr});
    WrappedStruct(const WrappedStruct& other);
    WrappedStruct(WrappedStruct&& other) noexcept;

    /**
     * @brief Assigns to the struct.
     * @note Copy only allowed if of the same type.
     *
     * @param other The other wrapped struct to copy/move from
     * @return A reference to this wrapped struct.
     */
    WrappedStruct& operator=(const WrappedStruct& other);
    WrappedStruct& operator=(WrappedStruct&& other) noexcept;

    /**
     * @brief Destroys the wrapped struct
     */
    ~WrappedStruct() = default;

    /**
     * @brief Sets a property on this struct
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
        return this->set<T>(this->type->find_prop_and_validate<T>(name), idx, value);
    }
    template <typename T>
    void set(const T* prop, size_t idx, const typename PropTraits<T>::Value& value) {
        set_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this->base.get()), value);
    }
};

/**
 * @brief Recursively copies all properties on a struct.
 *
 * @param dest The address of the struct to copy to.
 * @param src The source struct to copy from.
 */
void copy_struct(uintptr_t dest, const ReadOnlyWrappedStruct& src);

/**
 * @brief Recursively destroys all properties on a struct.
 *
 * @param type The type of the struct.
 * @param addr The address of the struct to destroy.
 */
void destroy_struct(const UStruct* type, uintptr_t addr);

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_WRAPPED_STRUCT_H */
