#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fweakobjectptr.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

#ifdef UE4
#include "unrealsdk/unreal/structs/gobjects.h"
#else
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/version_error.h"
#endif

namespace unrealsdk::unreal {

#pragma region Iterator

GObjects::Iterator::Iterator(const GObjects& gobjects, size_t idx)
    : gobjects(&gobjects), idx(idx) {}

GObjects::Iterator::reference GObjects::Iterator::operator*() const {
    return this->gobjects->obj_at(idx);
}

GObjects::Iterator& GObjects::Iterator::operator++() {
    do {
        // If we're on the last object, increment to max index
        if (this->idx >= (this->gobjects->size() - 1)) {
            this->idx = std::numeric_limits<size_t>::max();
            break;
        }

        ++this->idx;

        // If this index points to a null object, increment again
        // We really should handle gc'd object entries better (in UE4), but this is a quick hack to
        // get the iterator mostly working. In practice, you really shouldn't be iterating through
        // all objects anyway.
    } while (this->operator*() == nullptr);

    return *this;
}
GObjects::Iterator GObjects::Iterator::operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
}

bool GObjects::Iterator::operator==(const GObjects::Iterator& rhs) const {
    return this->idx == rhs.idx;
};
bool GObjects::Iterator::operator!=(const GObjects::Iterator& rhs) const {
    return !(*this == rhs);
};

GObjects::Iterator GObjects::begin(void) const {
    return {*this, 0};
}

GObjects::Iterator GObjects::end(void) const {
    return {*this, std::numeric_limits<size_t>::max()};
}

#pragma endregion

GObjects::GObjects(void) : internal(nullptr) {}
GObjects::GObjects(internal_type internal) : internal(internal) {}

#if defined(UE4)

size_t GObjects::size(void) const {
    return this->internal->ObjObjects.Count;
}

UObject* GObjects::obj_at(size_t idx) const {
    if (idx > (size_t)this->internal->ObjObjects.Count) {
        throw std::out_of_range("GObjects index out of range");
    }
    return this->internal->ObjObjects.at(idx)->Object;
}

UObject* GObjects::get_weak_object(const FWeakObjectPtr* ptr) const {
    if (ptr->object_serial_number == 0) {
        return nullptr;
    }
    if (ptr->object_index < 0) {
        return nullptr;
    }

    auto obj_item = this->internal->ObjObjects.at(ptr->object_index);
    if (ptr->object_serial_number != obj_item->SerialNumber) {
        return nullptr;
    }

    return obj_item->Object;
}

void GObjects::set_weak_object(FWeakObjectPtr* ptr, const UObject* obj) const {
    if (obj == nullptr) {
        ptr->object_index = -1;
        ptr->object_serial_number = 0;
    } else {
        ptr->object_index = obj->InternalIndex;

        auto obj_item = this->internal->ObjObjects.at(ptr->object_index);
        auto serial_number = obj_item->SerialNumber.load();

        // If we don't have a serial number
        if (serial_number == 0) {
            // Get a new one
            serial_number = ++this->internal->MasterSerialNumber;

            // Try write it
            // Another thread may have done so before we finish, need a compare exchange
            int32_t old_serial_number = 0;
            if (!obj_item->SerialNumber.compare_exchange_strong(old_serial_number, serial_number)) {
                serial_number = old_serial_number;
            }
        }

        ptr->object_serial_number = serial_number;
    }
}

#elif defined(UE3)

size_t GObjects::size(void) const {
    return this->internal->size();
}

UObject* GObjects::obj_at(size_t idx) const {
    return this->internal->at(idx);
}

UObject* GObjects::get_weak_object(const FWeakObjectPtr* /* ptr */) const {
    (void)this;
    throw_version_error("Weak object pointers are not implemented in UE3");
    return nullptr;
}

void GObjects::set_weak_object(FWeakObjectPtr* /* ptr */, const UObject* /* obj */) const {
    (void)this;
    throw_version_error("Weak object pointers are not implemented in UE3");
}

#else
#error Unknown UE version
#endif

}  // namespace unrealsdk::unreal
