#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/unreal/structs/fstring.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"
#include "unrealsdk/version_error.h"

namespace unrealsdk::unreal {

#ifdef UE4

FText::FText(std::string_view str) : FText(utils::widen(str)) {}
FText::FText(std::wstring_view str) : data(), flags() {
    unrealsdk::ftext_as_culture_invariant(this, str);
}

FText& FText::operator=(std::string_view str) {
    return *this = utils::widen(str);
}
FText& FText::operator=(std::wstring_view str) {
    // Modifying in place is a pain, instead create a new FText and swap it in
    FText local_text{str};
    std::swap(this->data, local_text.data);
    std::swap(this->flags, local_text.flags);

    // The old value in this is now swapped into local text, so will get deleted on leaving scope

    return *this;
}

FText::operator std::string() const {
    return utils::narrow(this->operator std::wstring_view());
}
FText::operator std::wstring() const {
    return std::wstring{this->operator std::wstring_view()};
}
FText::operator std::wstring_view() const {
    static auto idx = env::get_numeric<size_t>(env::FTEXT_GET_DISPLAY_STRING_VF_INDEX,
                                               env::defaults::FTEXT_GET_DISPLAY_STRING_VF_INDEX);

    auto text_data = this->data.obj;
    if (text_data == nullptr) {
        return L"";
    }

    return *reinterpret_cast<UnmanagedFString* (*)(FTextData*)>(text_data->vftable[idx])(text_data);
}

FText::~FText() {
    if (this->data.controller != nullptr) {
        this->data.controller->remove_strong_ref();
    }
}

#else

FText::FText(std::string_view /* str */) : data(), flags() {
    (void)this;
    (void)this->data;
    (void)this->flags;
    throw_version_error("FTexts are not implemented in UE3");
}
FText::FText(std::wstring_view /* str */) : data(), flags() {
    (void)this;
    throw_version_error("FTexts are not implemented in UE3");
}
FText& FText::operator=(std::string_view /* str */) {
    (void)this;
    throw_version_error("FTexts are not implemented in UE3");
    return *this;
}
FText& FText::operator=(std::wstring_view /* str */) {
    (void)this;
    throw_version_error("FTexts are not implemented in UE3");
    return *this;
}
FText::operator std::string() const {
    (void)this;
    throw_version_error("FTexts are not implemented in UE3");
    return {};
}
FText::operator std::wstring() const {
    (void)this;
    throw_version_error("FTexts are not implemented in UE3");
    return {};
}
FText::operator std::wstring_view() const {
    (void)this;
    throw_version_error("FTexts are not implemented in UE3");
    return {};
}
// Rather not throw from a destructor, since it will crash the whole game
FText::~FText() {
    (void)this;
}

#endif

}  // namespace unrealsdk::unreal
