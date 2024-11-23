#include "unrealsdk/pch.h"

#include "unrealsdk/config.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

#ifdef UE3

size_t UProperty::class_size(void) {
    static size_t size = 0;
    if (size != 0) {
        return size;
    }

    size = config::get().uproperty_size;
    if (size != 0) {
        return size;
    }

    // Rather than bother with a findobject call, we can recover UProperty from any arbitrary object
    // UObject always has properties on it, we don't need to worry about what class we get
    auto obj = *unrealsdk::gobjects().begin();
    auto prop = obj->Class->PropertyLink;

    const UStruct* cls = nullptr;
    for (auto superfield : prop->Class->superfields()) {
        if (superfield->Name == L"Property"_fn) {
            cls = superfield;
            break;
        }
    }

    // If we couldn't find the class, default to our actual size
    if (cls == nullptr) {
        size = sizeof(UProperty);
        LOG(WARNING, "Couldn't find UProperty class size, defaulting to: {:#x}", size);
    } else {
        size = cls->get_struct_size();
        LOG(MISC, "UProperty class size: {:#x}", size);
    }
    return size;
}

#endif

}  // namespace unrealsdk::unreal
