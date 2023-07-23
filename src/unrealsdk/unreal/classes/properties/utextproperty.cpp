#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/utextproperty.h"
#include "unrealsdk/env.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unrealsdk.h"

#ifdef UE4

namespace unrealsdk::unreal {

PropTraits<UTextProperty>::Value PropTraits<UTextProperty>::get(
    const UTextProperty* /*prop*/,
    uintptr_t addr,
    const UnrealPointer<void>& /*parent*/) {
    auto text_data = reinterpret_cast<FText*>(addr)->data.obj;

    static auto idx = env::get_numeric<size_t>(env::FTEXT_GET_DISPLAY_STRING_VF_INDEX,
                                               env::defaults::FTEXT_GET_DISPLAY_STRING_VF_INDEX);
    return *reinterpret_cast<UnmanagedFString* (*)(FTextData*)>(text_data->vftable[idx])(text_data);
}

void PropTraits<UTextProperty>::set(const UTextProperty* /*prop*/,
                                    uintptr_t addr,
                                    const Value& value) {
    auto text = reinterpret_cast<FText*>(addr);

    // Modifying in place is going to be a pain
    // Instead, create a new FText, and swap it
    FText local_text{};
    unrealsdk::ftext_as_culture_invariant(&local_text, value);
    std::swap(*text, local_text);

    // Destroy the old text, which is now swapped into our local var
    if (local_text.data.controller != nullptr) {
        local_text.data.controller->remove_strong_ref();
    }
}

void PropTraits<UTextProperty>::destroy(const UTextProperty* /*prop*/, uintptr_t addr) {
    auto text = reinterpret_cast<FText*>(addr);
    if (text->data.controller != nullptr) {
        text->data.controller->remove_strong_ref();
    }
}

}  // namespace unrealsdk::unreal

#endif
