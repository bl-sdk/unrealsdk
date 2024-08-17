#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/wrappers/weak_pointer.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

#ifdef UNREALSDK_EMULATED_WEAK_POINTER
WeakPointer::WeakPointer(const UObject* obj) : WeakPointer() {
    *this = obj;
}

WeakPointer& WeakPointer::operator=(const UObject* obj) {
    this->obj_addr = reinterpret_cast<uintptr_t>(obj);
    if (obj != nullptr) {
        this->index = obj->InternalIndex;

        this->outer_addr = reinterpret_cast<uintptr_t>(obj->Outer);
        this->vftable_addr = reinterpret_cast<uintptr_t>(obj->vftable);
        this->class_addr = reinterpret_cast<uintptr_t>(obj->Class);
        this->name = obj->Name;
    }
    return *this;
}

UObject* WeakPointer::operator*(void) const {
    if (this->obj_addr == 0) {
        return nullptr;
    }
    auto gobjects = unrealsdk::gobjects();
    if (this->index >= gobjects.size()) {
        return nullptr;
    }
    auto obj = gobjects.obj_at(this->index);
    if (obj == nullptr || this->obj_addr != reinterpret_cast<uintptr_t>(obj)) {
        return nullptr;
    }

    if (this->outer_addr != reinterpret_cast<uintptr_t>(obj->Outer)
        || this->vftable_addr != reinterpret_cast<uintptr_t>(obj->vftable)
        || this->class_addr != reinterpret_cast<uintptr_t>(obj->Class) || this->name != obj->Name) {
        return nullptr;
    }

    return obj;
}

#else
WeakPointer::WeakPointer(const UObject* obj) {
    unrealsdk::gobjects().set_weak_object(&this->ptr, obj);
}

WeakPointer& WeakPointer::operator=(const UObject* obj) {
    unrealsdk::gobjects().set_weak_object(&this->ptr, obj);
    return *this;
}

UObject* WeakPointer::operator*(void) const {
    return unrealsdk::gobjects().get_weak_object(&this->ptr);
}
#endif

UObject* WeakPointer::operator*(void) {
    // Get the result of the const dereference
    auto ret = **((const WeakPointer*)this);

    // If we have a mutable reference, and the object is null, we can set ourselves to null, we'll
    // never have a valid object reference again. This is generally an optimization, since we can
    // early exit.
    // In UE3 particularly, this also further reduces the chances of us returning an incorrect
    // object, assuming you're regularly accessing the pointer.
    if (ret == nullptr) {
        *this = nullptr;
    }
    return ret;
}

WeakPointer::operator bool(void) {
    return **this != nullptr;
}
WeakPointer::operator bool(void) const {
    return **this != nullptr;
}

}  // namespace unrealsdk::unreal
