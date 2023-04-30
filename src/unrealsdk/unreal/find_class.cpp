#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

namespace {

#ifndef UNREALSDK_IMPORTING
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
#endif

}  // namespace

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI [[nodiscard]] UClass* find_class_fname(const FName* name) noexcept;
#endif
#ifdef UNREALSDK_IMPORTING
UClass* find_class(const FName& name) noexcept {
    return find_class_fname(&name);
}
#else
UClass* find_class(const FName& name) noexcept {
    if (cache.empty()) {
        initialize_cache();
    }

    if (!cache.contains(name)) {
        return nullptr;
    }

    return cache[name];
}
#endif
#ifdef UNREALSDK_EXPORTING
UClass* find_class_fname(const FName* name) noexcept {
    return find_class(*name);
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI [[nodiscard]] UClass* find_class_cstr(const wchar_t* name,
                                                     size_t name_size) noexcept;
#endif
#ifdef UNREALSDK_IMPORTING
UClass* find_class(const std::wstring& name) noexcept {
    return find_class_cstr(name.c_str(), name.size());
}
#else
UClass* find_class(const std::wstring& name) noexcept {
    if (cache.empty()) {
        initialize_cache();
    }

    auto cls = validate_type<UClass>(unrealsdk::find_object(uclass_class, name));
    if (cls == nullptr) {
        return nullptr;
    }

    if (!cache.contains(cls->Name)) {
        cache[cls->Name] = cls;
    }

    return cls;
}
#endif
#ifdef UNREALSDK_EXPORTING
UClass* find_class_cstr(const wchar_t* name, size_t name_size) noexcept {
    const std::wstring str{name, name_size};
    return find_class(str);
}
#endif

}  // namespace unrealsdk::unreal
