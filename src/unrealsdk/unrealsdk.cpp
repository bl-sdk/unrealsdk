#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/game/abstract_hook.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/logging.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/version.h"

namespace unrealsdk {

namespace {

#ifndef UNREALSDK_IMPORTING
constexpr auto LOG_FILE_NAME = "unrealsdk.log";

std::mutex mutex{};
std::unique_ptr<game::AbstractHook> hook_instance;
#endif

}  // namespace

#ifndef UNREALSDK_IMPORTING
bool init(std::unique_ptr<game::AbstractHook>&& game) {
    const std::lock_guard<std::mutex> lock(mutex);

    if (hook_instance != nullptr) {
        return false;
    }

    env::load_file();
    logging::init(LOG_FILE_NAME);

    auto version = unrealsdk::get_version_string();
    LOG(INFO, "{}", version);
    LOG(INFO, "{}", std::string(version.size(), '='));

    if (MH_Initialize() != MH_OK) {
        throw std::runtime_error("Minhook initialization failed!");
    }

    // Initialize the hook before moving it, to weed out any order of initialization problems.
    game->hook();

    hook_instance = std::move(game);
    return true;
}

bool is_initialized(void) {
    const std::lock_guard<std::mutex> lock(mutex);
    return hook_instance != nullptr;
}
#endif

#pragma region Wrappers

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI [[nodiscard]] const GObjects* gobjects_ptr(void) UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
const GObjects& gobjects(void) {
    return *gobjects_ptr();
}
#else
const GObjects& gobjects(void) {
    return hook_instance->gobjects();
}
#endif
#ifdef UNREALSDK_EXPORTING
const GObjects* gobjects_ptr(void) {
    return &hook_instance->gobjects();
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI [[nodiscard]] const GNames* gnames_ptr(void) UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
const GNames& gnames(void) {
    return *gnames_ptr();
}
#else
const GNames& gnames(void) {
    return hook_instance->gnames();
}
#endif
#ifdef UNREALSDK_EXPORTING
const GNames* gnames_ptr(void) {
    return &hook_instance->gnames();
}
#endif

#ifndef UNREALSDK_IMPORTING
void fname_init(FName* name, const wchar_t* str, int32_t number) {
    hook_instance->fname_init(name, str, number);
}
#endif
void fname_init(FName* name, const std::wstring& str, int32_t number) {
    fname_init(name, str.c_str(), number);
}

#ifndef UNREALSDK_IMPORTING
void fframe_step(FFrame* frame, UObject* obj, void* param) {
    hook_instance->fframe_step(frame, obj, param);
}

void* u_malloc(size_t len) {
    return hook_instance->u_malloc(len);
}
void* u_realloc(void* original, size_t len) {
    return hook_instance->u_realloc(original, len);
}
void u_free(void* data) {
    return hook_instance->u_free(data);
}

void process_event(UObject* object, UFunction* function, void* params) {
    hook_instance->process_event(object, function, params);
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI [[nodiscard]] UObject* construct_object(UClass* cls,
                                                       UObject* outer,
                                                       const FName* name = nullptr,
                                                       decltype(UObject::ObjectFlags) flags = 0,
                                                       UObject* template_obj = nullptr)
    UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
UObject* construct_object(UClass* cls,
                          UObject* outer,
                          const FName& name,
                          decltype(UObject::ObjectFlags) flags,
                          UObject* template_obj) {
    return construct_object(cls, outer, &name, flags, template_obj);
}
#else
UObject* construct_object(UClass* cls,
                          UObject* outer,
                          const FName& name,
                          decltype(UObject::ObjectFlags) flags,
                          UObject* template_obj) {
    return hook_instance->construct_object(cls, outer, name, flags, template_obj);
}
#endif
#ifdef UNREALSDK_EXPORTING
UObject* construct_object(UClass* cls,
                          UObject* outer,
                          const FName* name,
                          decltype(UObject::ObjectFlags) flags,
                          UObject* template_obj) {
    FName local_name{0, 0};
    if (name != nullptr) {
        local_name = *name;
    }
    return hook_instance->construct_object(cls, outer, local_name, flags, template_obj);
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI void uconsole_output_text(const wchar_t* str, size_t size) UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
void uconsole_output_text(const std::wstring& str) {
    uconsole_output_text(str.c_str(), str.size());
}
#else
void uconsole_output_text(const std::wstring& str) {
    // Since this is called by logging, which we know will happen plenty before/during
    // initialization, just drop any messages which arrive before we have a hook to work with
    if (hook_instance) {
        hook_instance->uconsole_output_text(str);
    }
}
#endif
#ifdef UNREALSDK_EXPORTING
void uconsole_output_text(const wchar_t* str, size_t size) {
    uconsole_output_text({str, size});
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI [[nodiscard]] wchar_t* uobject_path_name_cstr(const UObject* obj)
    UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
std::wstring uobject_path_name(const UObject* obj) {
    auto ptr = uobject_path_name_cstr(obj);
    std::wstring str{ptr};
    u_free(ptr);
    return str;
}
#else
std::wstring uobject_path_name(const UObject* obj) {
    return hook_instance->uobject_path_name(obj);
}
#endif
#ifdef UNREALSDK_EXPORTING
wchar_t* uobject_path_name_cstr(const UObject* obj) {
    auto name = hook_instance->uobject_path_name(obj);
    auto size = name.size();

    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
    auto mem = reinterpret_cast<wchar_t*>(u_malloc((size + 1) * sizeof(wchar_t)));
    wcsncpy_s(mem, size + 1, name.data(), size);

    return mem;
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI [[nodiscard]] UObject* find_object(UClass* cls,
                                                  const wchar_t* name,
                                                  size_t name_size) UNREALSDK_CAPI_SUFFIX;
#endif
#ifdef UNREALSDK_IMPORTING
UObject* find_object(UClass* cls, const std::wstring& name) {
    return find_object(cls, name.c_str(), name.size());
}
UObject* find_object(const FName& cls, const std::wstring& name) {
    return find_object(find_class(cls), name.c_str(), name.size());
}
UObject* find_object(const std::wstring& cls, const std::wstring& name) {
    return find_object(find_class(cls), name.c_str(), name.size());
}
#else
UObject* find_object(UClass* cls, const std::wstring& name) {
    return hook_instance->find_object(cls, name);
}
UObject* find_object(const FName& cls, const std::wstring& name) {
    return hook_instance->find_object(find_class(cls), name);
}
UObject* find_object(const std::wstring& cls, const std::wstring& name) {
    return hook_instance->find_object(find_class(cls), name);
}
#endif
#ifdef UNREALSDK_EXPORTING
UNREALSDK_CAPI [[nodiscard]] UObject* find_object(UClass* cls,
                                                  const wchar_t* name,
                                                  size_t name_size) UNREALSDK_CAPI_SUFFIX {
    return hook_instance->find_object(cls, {name, name_size});
}
#endif

}  // namespace unrealsdk
