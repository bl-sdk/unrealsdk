#include "pch.h"

#include "game/abstract_hook.h"
#include "hook_manager.h"
#include "logging.h"
#include "unreal/find_class.h"
#include "unrealsdk.h"
#include "version.h"

namespace unrealsdk {

namespace {

constexpr auto LOG_FILE_NAME = "unrealsdk.log";

std::unique_ptr<game::AbstractHook> hook_instance;

}  // namespace

void init(std::unique_ptr<game::AbstractHook> game) {
    logging::init(LOG_FILE_NAME);

    auto init_str = unrealsdk::fmt::format("unrealsdk {}", VERSION_STR);
    LOG(INFO, "{}", init_str);
    LOG(INFO, "{}", std::string(init_str.size(), '='));

    if (MH_Initialize() != MH_OK) {
        throw std::runtime_error("Minhook initialization failed!");
    }

    // Initialize the hook before moving it, to weed out any order of initialization problems.
    game->hook();

    hook_instance = std::move(game);
}

#pragma region Wrappers

const GObjects& gobjects(void) {
    return hook_instance->gobjects();
}
const GNames& gnames(void) {
    return hook_instance->gnames();
}
void fname_init(FName* name, const std::wstring& str, int32_t number) {
    hook_instance->fname_init(name, str.c_str(), number);
}
void fname_init(FName* name, const wchar_t* str, int32_t number) {
    hook_instance->fname_init(name, str, number);
}
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
UObject* construct_object(UClass* cls,
                          UObject* outer,
                          const FName& name,
                          decltype(UObject::ObjectFlags) flags,
                          UObject* template_obj) {
    return hook_instance->construct_object(cls, outer, name, flags, template_obj);
}
void uconsole_output_text(const std::wstring& str) {
    // Since this is called by logging, which we know will happen plenty before/during
    // initialization, just drop any messages which arrive before we have a hook to work with
    if (hook_instance) {
        hook_instance->uconsole_output_text(str);
    }
}
std::wstring uobject_path_name(const UObject* obj) {
    return hook_instance->uobject_path_name(obj);
}
UObject* find_object(UClass* cls, const std::wstring& name) {
    return hook_instance->find_object(cls, name);
}
UObject* find_object(const FName& cls, const std::wstring& name) {
    return hook_instance->find_object(find_class(cls), name);
}
UObject* find_object(const std::wstring& cls, const std::wstring& name) {
    return hook_instance->find_object(find_class(cls), name);
}

}  // namespace unrealsdk
