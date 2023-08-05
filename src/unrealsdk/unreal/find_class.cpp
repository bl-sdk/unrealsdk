#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/namedobjectcache.h"

namespace unrealsdk::unreal {

namespace {

#ifndef UNREALSDK_IMPORTING
class NamedClassCache : public NamedObjectCache<UClass> {
    [[nodiscard]] UClass* find_uclass(void) const override {
        // We can't exactly do a find class lookup here
        // Instead, just follow the class chain off of an arbitrary object
        // UClass is the only object whose class is itself
        auto cls = (*unrealsdk::gobjects().begin())->Class;
        for (; cls->Class != cls; cls = cls->Class) {}
        return cls;
    }
};

NamedClassCache cache;
#endif

}  // namespace

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI([[nodiscard]] UClass*, find_class_fname, const FName* name);
#endif
#ifdef UNREALSDK_IMPORTING
UClass* find_class(const FName& name) {
    return UNREALSDK_MANGLE(find_class_fname)(&name);
}
#else
UClass* find_class(const FName& name) {
    return cache.find(name);
}
#endif
#ifdef UNREALSDK_EXPORTING
UNREALSDK_CAPI([[nodiscard]] UClass*, find_class_fname, const FName* name) {
    return find_class(*name);
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI([[nodiscard]] UClass*, find_class_cstr, const wchar_t* name, size_t name_size);
#endif
#ifdef UNREALSDK_IMPORTING
UClass* find_class(const std::wstring& name) {
    return UNREALSDK_MANGLE(find_class_cstr)(name.c_str(), name.size());
}
#else
UClass* find_class(const std::wstring& name) {
    return cache.find(name);
}
#endif
#ifdef UNREALSDK_EXPORTING
UNREALSDK_CAPI([[nodiscard]] UClass*, find_class_cstr, const wchar_t* name, size_t name_size) {
    return find_class(std::wstring{name, name_size});
}
#endif

}  // namespace unrealsdk::unreal
