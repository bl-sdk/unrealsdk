#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/find_class.h"

#include "unrealsdk/unrealsdk_fw.inl"
/*
====================================================================================================

This file contains all the C++ wrappers around the C api functions, the functions which are actually
exposed in the header.

====================================================================================================
*/

namespace unrealsdk {

[[nodiscard]] bool is_initialized(void) {
    return UNREALSDK_MANGLE(is_initialized)();
}

[[nodiscard]] bool is_console_ready(void) {
    return UNREALSDK_MANGLE(is_console_ready)();
}

const GObjects& gobjects(void) {
    return *UNREALSDK_MANGLE(gobjects)();
}

const GNames& gnames(void) {
    return *UNREALSDK_MANGLE(gnames)();
}

void fname_init(FName* name, const wchar_t* str, int32_t number) {
    UNREALSDK_MANGLE(fname_init)(name, str, number);
}
void fname_init(FName* name, std::wstring_view str, int32_t number) {
    UNREALSDK_MANGLE(fname_init)(name, str.data(), number);
}

void fframe_step(FFrame* frame, UObject* obj, void* param) {
    UNREALSDK_MANGLE(fframe_step(frame, obj, param));
}

void* u_malloc(size_t len) {
    return UNREALSDK_MANGLE(u_malloc)(len);
}
void* u_realloc(void* original, size_t len) {
    return UNREALSDK_MANGLE(u_realloc)(original, len);
}
void u_free(void* data) {
    UNREALSDK_MANGLE(u_free)(data);
}

void process_event(UObject* object, UFunction* function, void* params) {
    UNREALSDK_MANGLE(process_event)(object, function, params);
}

UObject* construct_object(UClass* cls,
                          UObject* outer,
                          const FName& name,
                          decltype(UObject::ObjectFlags) flags,
                          UObject* template_obj) {
    return UNREALSDK_MANGLE(construct_object)(cls, outer, &name, flags, template_obj);
}

void uconsole_output_text(std::wstring_view str) {
    UNREALSDK_MANGLE(uconsole_output_text)(str.data(), str.size());
}

std::wstring uobject_path_name(const UObject* obj) {
    size_t size{};
    auto ptr = UNREALSDK_MANGLE(uobject_path_name)(obj, size);

    std::wstring str{ptr, size};
    u_free(ptr);
    return str;
}

UObject* find_object(UClass* cls, std::wstring_view name) {
    return UNREALSDK_MANGLE(find_object)(cls, name.data(), name.size());
}
UObject* find_object(const FName& cls, std::wstring_view name) {
    return UNREALSDK_MANGLE(find_object)(find_class(cls), name.data(), name.size());
}
UObject* find_object(std::wstring_view cls, std::wstring_view name) {
    return UNREALSDK_MANGLE(find_object)(find_class(cls), name.data(), name.size());
}

#ifdef UE4

void ftext_as_culture_invariant(unreal::FText* text, unreal::TemporaryFString&& str) {
    UNREALSDK_MANGLE(ftext_as_culture_invariant)(text, std::move(str));
}

#endif

}  // namespace unrealsdk
