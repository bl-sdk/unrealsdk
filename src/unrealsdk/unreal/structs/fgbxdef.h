#ifndef UNREALSDK_UNREAL_STRUCTS_FGBXDEF_H
#define UNREALSDK_UNREAL_STRUCTS_FGBXDEF_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

/*
There are a number of custom types in BL4, which all revolve around this one:
- FGameDataHandle
- ZGameDataHandleProperty
- FGbxDefPtr
- ZGbxDefPtrProperty
- FGbxInlineStruct
- ZGbxInlineStructProperty

I highly suspect these are LLM slop. There's nothing unique these do, they're all questionable
reimplementations of existing engine features. The only "good" justification I can come up with is
they slowed down reverse engineering.

Like said before, this is the root type. A FGbxDef maps to a ScriptStruct inside the unrealscript
VM, and it's just *some struct of data*. The one thing they have in common is they all have a
virtual function table, which includes two (maybe three?) common functions. These are essentially
just a bad UObject reimplementation.

All defs are loaded from .ncs files inside the pak. This is another custom file format the LLMs
cooked up. They're essentially just a reimplementation of uasset files, which in memory are
essentially datatables.

FGameDataHandle and FGbxDefPtr appear to hold *non-owning* references into the datatables. They're
both essentially soft object pointers, just in two different forms. They both hold a type and a
name, which appear to uniquely identify a def. When the game accesses one of them, it resolves it to
an actual pointer to the struct. At this point we do not try to resolve them ourselves, like we do
with real soft object pointers, you'll have to find a resolved instance to edit it.

FGbxInlineStruct is a pointer to an explicitly out-of-line FGbxDef. This uses a shared pointer, so
in this instance they *are* owning references. Because of this we do let you create your own. Not
entirely sure how they relate to the NCS datatables.

ZGbxDefPtrProperty and ZGbxInlineStructProperty both appear totally ok with inheritance - you can
assign a child def class totally fine (just like UObjects). ZGameDataHandleProperty uses some sort
of type handle we haven't decoded yet, though so far it appears to only allow exact matches.
*/

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

struct FGbxDef {
    uintptr_t* vftable;
    // Note we always expect to be working with a subclass, which'll have more fields than this.

    /**
     * @brief Gets the type of this struct.
     *
     * @return The struct type.
     */
    [[nodiscard]] UScriptStruct* get_type(void) const;

    /**
     * @brief Creates a wrapped struct around this def.
     *
     * @param parent The parent pointer this struct was retrieved from, used to copy ownership.
     * @return The wrapped struct.
     */
    [[nodiscard]] WrappedStruct to_struct(const UnrealPointer<void>& parent = {nullptr});

    /**
     * @brief Gets a property on this def.
     *
     * @tparam T The type of the property.
     * @param name The property's name to lookup.
     * @param prop The property to get.
     * @param idx The fixed array index to get the value at. Defaults to 0.
     * @return The property's value.
     */
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(const FName& name, size_t idx = 0) const {
        return this->get<T>(this->get_type()->find_prop_and_validate<T>(name), idx);
    }
    template <typename T>
    [[nodiscard]] typename PropTraits<T>::Value get(const T* prop, size_t idx = 0) const {
        return get_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this));
    }

    /**
     * @brief Sets a property on this def.
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
        return this->set<T>(this->get_type()->find_prop_and_validate<T>(name), idx, value);
    }
    template <typename T>
    void set(const T* prop, size_t idx, const typename PropTraits<T>::Value& value) {
        set_property<T>(prop, idx, reinterpret_cast<uintptr_t>(this), value);
    }
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FGBXDEF_H */
