#include "pch.h"

#include "unreal/class_name.h"
#include "unreal/classes/uclass.h"
#include "unreal/find_class.h"
#include "unreal/wrappers/gobjects.h"
#include "unrealsdk.h"

namespace unrealsdk::unreal {

namespace {

// Cache of all known classes by FName
// We leave duplicate names undefined
std::unordered_map<FName, UClass*> cache;
// UClass is important enough to have a separate cached value of
UClass* uclass_class = nullptr;

void initialize_cache(void) {
    cache.clear();

    // Start by grabbing UClass, working off of an arbitrary object
    auto cls = (*unrealsdk::gobjects().begin())->Class;
    for (; cls->Class != cls; cls = cls->Class) {}
    uclass_class = cls;

    cache[uclass_class->Name] = uclass_class;

    // Then add all other classes we can find
    for (const auto& obj : unrealsdk::gobjects()) {
        if (obj == uclass_class) {
            continue;
        }
        if (!obj->is_instance(uclass_class)) {
            continue;
        }

        cache[obj->Name] = reinterpret_cast<UClass*>(obj);
    }
}

}  // namespace

UClass* find_class(const FName& name) {
    if (cache.empty()) {
        initialize_cache();
    }

    if (!cache.contains(name)) {
        throw std::runtime_error("Unknown class name: " + (std::string)name);
    }

    return cache[name];
}

UClass* find_class(const std::wstring& name) {
    if (cache.empty()) {
        initialize_cache();
    }

    auto cls = validate_type<UClass>(unrealsdk::find_object(uclass_class, name));
    if (cls == nullptr) {
        throw std::runtime_error("Could not find class: " + utils::narrow(name));
    }

    if (!cache.contains(cls->Name)) {
        cache[cls->Name] = cls;
    }

    return cls;
}

}  // namespace unrealsdk::unreal
