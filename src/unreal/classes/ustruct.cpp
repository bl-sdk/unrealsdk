#include "pch.h"

#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {


#pragma region Iterator

UStruct::PropertyIterator::PropertyIterator(UProperty* prop) : prop(prop) {}

UStruct::PropertyIterator::reference UStruct::PropertyIterator::operator*() const {
    return prop;
}

UStruct::PropertyIterator& UStruct::PropertyIterator::operator++() {
    prop = prop->PropertyLinkNext;
    return *this;
}
UStruct::PropertyIterator UStruct::PropertyIterator::operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
}

bool UStruct::PropertyIterator::operator==(const UStruct::PropertyIterator& rhs) const {
    return this->prop == rhs.prop;
};
bool UStruct::PropertyIterator::operator!=(const UStruct::PropertyIterator& rhs) const {
    return !(*this == rhs);
};

UStruct::PropertyIterator UStruct::begin(void) const {
    return {this->PropertyLink};
}
UStruct::PropertyIterator UStruct::end(void) {
    return {nullptr};
}

#pragma endregion

size_t UStruct::get_struct_size(void) const {
#ifdef UE4
    return (this->PropertySize + this->MinAlignment - 1) & ~(this->MinAlignment - 1);
#else
    return this->PropertySize;
#endif
}

}  // namespace unrealsdk::unreal
