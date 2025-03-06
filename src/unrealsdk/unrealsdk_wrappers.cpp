#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unrealsdk.h"

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

void* u_malloc(size_t len) {
    return UNREALSDK_MANGLE(u_malloc)(len);
}
void* u_realloc(void* original, size_t len) {
    return UNREALSDK_MANGLE(u_realloc)(original, len);
}
void u_free(void* data) {
    UNREALSDK_MANGLE(u_free)(data);
}

UObject* construct_object(UClass* cls,
                          UObject* outer,
                          const FName& name,
                          uint64_t flags,
                          UObject* template_obj) {
    return UNREALSDK_MANGLE(construct_object)(cls, outer, &name, flags, template_obj);
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

UObject* load_package(std::wstring_view name, uint32_t flags) {
    return UNREALSDK_MANGLE(load_package)(name.data(), name.size(), flags);
}

namespace internal {

void fname_init(FName* name, const wchar_t* str, int32_t number) {
    UNREALSDK_MANGLE(fname_init)(name, str, number);
}
void fname_init(FName* name, const std::wstring& str, int32_t number) {
    UNREALSDK_MANGLE(fname_init)(name, str.data(), number);
}

void fframe_step(FFrame* frame, UObject* obj, void* param) {
    UNREALSDK_MANGLE(fframe_step(frame, obj, param));
}

void process_event(UObject* object, UFunction* function, void* params) {
    UNREALSDK_MANGLE(process_event)(object, function, params);
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

void ftext_as_culture_invariant(unreal::FText* text, unreal::TemporaryFString&& str) {
    UNREALSDK_MANGLE(ftext_as_culture_invariant)(text, std::move(str));
}

void fsoftobjectptr_assign(unreal::FSoftObjectPtr* ptr, const unreal::UObject* obj) {
    UNREALSDK_MANGLE(fsoftobjectptr_assign)(ptr, obj);
}

void flazyobjectptr_assign(unreal::FLazyObjectPtr* ptr, const unreal::UObject* obj) {
    UNREALSDK_MANGLE(flazyobjectptr_assign)(ptr, obj);
}

[[nodiscard]] const offsets::OffsetList& get_offsets() {
    return *UNREALSDK_MANGLE(get_offsets)();
}

[[nodiscard]] unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> uenum_get_names(
    const unreal::UEnum* uenum) {
    auto ret = UNREALSDK_MANGLE(uenum_get_names)(uenum);

    unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> names{
        .data = nullptr, .count = 0, .max = 0};
    std::swap(names.count, ret.count);
    std::swap(names.max, ret.max);

    // different types so can't std::swap
    names.data = reinterpret_cast<unreal::TPair<unreal::FName, uint64_t>*>(ret.data);
    ret.data = nullptr;

    return names;
}

}  // namespace internal

}  // namespace unrealsdk
