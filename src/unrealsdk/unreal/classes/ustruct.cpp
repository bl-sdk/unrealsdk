#include "unrealsdk/pch.h"

#include "unrealsdk/config.h"
#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/utils.h"

namespace unrealsdk::unreal {

#ifdef UE3

size_t UStruct::class_size(void) {
    static size_t size = 0;
    if (size != 0) {
        return size;
    }

    auto config_size = config::get_int("unrealsdk.ustruct_size");
    if (config_size.has_value()) {
        size = (size_t)*config_size;
        return size;
    }

    // Rather than bother with a find object/class, we can recover UStruct from any arbitrary object
    // This just avoids extra dependencies, especially since theoretically find class might depend
    // on this

    // First, find UClass
    auto obj = *unrealsdk::gobjects().begin();
    const UClass* class_cls = obj->Class;
    for (; class_cls->Class != class_cls; class_cls = class_cls->Class) {}

    // Then look through it's superfields for UStruct
    const UStruct* struct_cls = nullptr;
    for (auto superfield : class_cls->superfields()) {
        if (superfield->Name == L"Struct"_fn) {
            struct_cls = superfield;
            break;
        }
    }

    // If we couldn't find the class, default to our actual size
    if (struct_cls == nullptr) {
        size = sizeof(UStruct);
        LOG(WARNING, "Couldn't find UStruct class size, defaulting to: {:#x}", size);
    } else {
        size = struct_cls->get_struct_size();
        LOG(MISC, "UStruct class size: {:#x}", size);
    }
    return size;
}

#endif

#pragma region Field Iterator

UStruct::FieldIterator::FieldIterator(void) : this_struct(nullptr), field(nullptr) {}
UStruct::FieldIterator::FieldIterator(const UStruct* this_struct, UField* field)
    : this_struct(this_struct), field(field) {}

UStruct::FieldIterator::reference UStruct::FieldIterator::operator*() const {
    return this->field;
}

UStruct::FieldIterator& UStruct::FieldIterator::operator++() {
    if (this->field != nullptr) {
        this->field = this->field->Next;
    }
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

utils::IteratorProxy<UStruct::FieldIterator> UStruct::fields(void) const {
    FieldIterator begin{this, this->Children};

    // If we start out pointing at null (because this struct has no direct children), increment once
    //  to find the actual first field
    if (*begin == nullptr) {
        begin++;
    }

    return {begin, {}};
}

#pragma endregion

#pragma region Property Iterator

UStruct::PropertyIterator::PropertyIterator(void) : prop(nullptr) {}
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

utils::IteratorProxy<UStruct::PropertyIterator> UStruct::properties(void) const {
    return {{this->PropertyLink}, {}};
}

#pragma endregion

#pragma region SuperField Iterator

UStruct::SuperFieldIterator::SuperFieldIterator(void) : this_struct(nullptr) {}
UStruct::SuperFieldIterator::SuperFieldIterator(const UStruct* this_struct)
    : this_struct(this_struct) {}

UStruct::SuperFieldIterator::reference UStruct::SuperFieldIterator::operator*() const {
    return this->this_struct;
}

UStruct::SuperFieldIterator& UStruct::SuperFieldIterator::operator++() {
    this->this_struct = this->this_struct->SuperField;
    return *this;
}
UStruct::SuperFieldIterator UStruct::SuperFieldIterator::operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
}

bool UStruct::SuperFieldIterator::operator==(const UStruct::SuperFieldIterator& rhs) const {
    return this->this_struct == rhs.this_struct;
};
bool UStruct::SuperFieldIterator::operator!=(const UStruct::SuperFieldIterator& rhs) const {
    return !(*this == rhs);
};

utils::IteratorProxy<UStruct::SuperFieldIterator> UStruct::superfields(void) const {
    return {{this}, {}};
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
    for (auto field : this->fields()) {
        if (field->Name == name) {
            return field;
        }
    }

    throw std::invalid_argument("Couldn't find field " + (std::string)name);
}

UProperty* UStruct::find_prop(const FName& name) const {
    for (auto prop : this->properties()) {
        if (prop->Name == name) {
            return prop;
        }
    }

    throw std::invalid_argument("Couldn't find property " + (std::string)name);
}

UFunction* UStruct::find_func_and_validate(const FName& name) const {
    return validate_type<UFunction>(this->find(name));
}

bool UStruct::inherits(const UStruct* base_struct) const {
    auto superfields = this->superfields();
    return std::ranges::find(superfields, base_struct) != superfields.end();
}

}  // namespace unrealsdk::unreal
