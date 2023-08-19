#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

#ifdef UE4

namespace unrealsdk::unreal {

FText::FText(const std::string& str) : FText(utils::widen(str)) {}
FText::FText(const std::wstring& str) : data(), flags() {
    unrealsdk::ftext_as_culture_invariant(this, str);
}

FText& FText::operator=(const std::string& str) noexcept {
    return *this = utils::widen(str);
}
FText& FText::operator=(const std::wstring& str) noexcept {
    // Modifying in place is a pain, instead create a new FText and swap it in
    FText local_text{str};
    std::swap(this->data, local_text.data);
    std::swap(this->flags, local_text.flags);

    // The old value in this is now swapped into local text, so will get deleted on leaving scope

    return *this;
}

FText::operator std::string() const {
    static auto idx = env::get_numeric<size_t>(env::FTEXT_GET_DISPLAY_STRING_VF_INDEX,
                                               env::defaults::FTEXT_GET_DISPLAY_STRING_VF_INDEX);

    auto text_data = this->data.obj;
    return *reinterpret_cast<UnmanagedFString* (*)(FTextData*)>(text_data->vftable[idx])(text_data);
}
FText::operator std::wstring() const {
    return utils::widen(this->operator std::string());
}

FText::~FText() {
    if (this->data.controller != nullptr) {
        this->data.controller->remove_strong_ref();
    }
}

}  // namespace unrealsdk::unreal

#endif
