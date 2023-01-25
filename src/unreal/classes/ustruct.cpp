#include "pch.h"

#include "unreal/classes/ufield.h"
#include "unreal/classes/ustruct.h"

namespace unrealsdk::unreal {

#pragma region Iterator

UStruct::FieldIterator::FieldIterator(const UStruct* this_struct, UField* field)
    : this_struct(this_struct), field(field) {}

UStruct::FieldIterator::reference UStruct::FieldIterator::operator*() const {
    return this->field;
}

UStruct::FieldIterator& UStruct::FieldIterator::operator++() {
    this->field = this->field->Next;
    while (this->field == nullptr && this->this_struct != nullptr) {
        this->this_struct = this->this_struct->SuperField;

        if (this->this_struct != nullptr) {
            this->field = this->this_struct->Children;
        }
    }

    return *this;
}
UStruct::FieldIterator UStruct::FieldIterator::operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
}

bool UStruct::FieldIterator::operator==(const UStruct::FieldIterator& rhs) const {
    return this->this_struct == rhs.this_struct && this->field == rhs.field;
};
bool UStruct::FieldIterator::operator!=(const UStruct::FieldIterator& rhs) const {
    return !(*this == rhs);
};

UStruct::FieldIterator UStruct::begin(void) const {
    return {this, this->Children};
}
UStruct::FieldIterator UStruct::end(void) {
    return {nullptr, nullptr};
}

#pragma endregion

size_t UStruct::get_struct_size(void) const {
#ifdef UE4
    return (this->PropertySize + this->MinAlignment - 1) & ~(this->MinAlignment - 1);
#else
    return this->PropertySize;
#endif
}

UField* UStruct::find(const FName& name) const {
    for (auto field : *this) {
        if (field->Name == name) {
            return field;
        }
    }

    throw std::invalid_argument("Couldn't find property " + (std::string)name);
}

}  // namespace unrealsdk::unreal
