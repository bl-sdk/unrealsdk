#include "pch.h"

#include "unreal/classes/uobject.h"
#include "unreal/wrappers/gobjects.h"

#ifdef UE4
#include "unreal/structs/gobjects.h"
#else
#include "unreal/structs/tarray.h"
#endif

namespace unrealsdk::unreal {

#pragma region Iterator

GObjects::Iterator::Iterator(const GObjects& gobjects, size_t idx) : gobjects(gobjects), idx(idx) {}

GObjects::Iterator::reference GObjects::Iterator::operator*() const {
    return this->gobjects.obj_at(idx);
}

GObjects::Iterator& GObjects::Iterator::operator++() {
    do {
        // If we're on the last object, increment to max index
        if (this->idx >= (this->gobjects.size() - 1)) {
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

#elif defined(UE3)

size_t GObjects::size(void) const {
    return this->internal->size();
}

UObject* GObjects::obj_at(size_t idx) const {
    return this->internal->at(idx);
}

#else
#error Unknown UE version
#endif

}  // namespace unrealsdk::unreal
