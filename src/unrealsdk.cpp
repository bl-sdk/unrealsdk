#include "pch.h"

#include "game/abstract_hook.h"
#include "hook_manager.h"
#include "logging.h"
#include "unrealsdk.h"
#include "version.h"

namespace unrealsdk {

static std::unique_ptr<game::AbstractHook> hook_instance;

void init(std::unique_ptr<game::AbstractHook> game) {
    logging::init();

    auto init_str = unrealsdk::fmt::format("unrealsdk {}", VERSION_STR);
    LOG(INFO, "{}", init_str);
    LOG(INFO, "{}", std::string(init_str.size(), '='));

    if (MH_Initialize() != MH_OK) {
        throw std::runtime_error("Minhook initialization failed!");
    }

    // Initalize the hook before moving it, to weed out any order of initalization problems.
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
    hook_instance->fname_init(name, str, number);
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
    // Since this is called so often, do some actual error checking to make sure the hook exists
    if (hook_instance) {
        hook_instance->uconsole_output_text(str);
    }
}
std::wstring uobject_path_name(const UObject* obj) {
    return hook_instance->uobject_path_name(obj);
}

}  // namespace unrealsdk
