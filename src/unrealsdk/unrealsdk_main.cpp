#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/game/abstract_hook.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/logging.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/version.h"

#include "unrealsdk/unrealsdk_fw.inl"
/*
====================================================================================================

This file contains the SDK initialization and the static game hook pointer, and the base C api
functions (which reference the game hook).

====================================================================================================
*/

#ifndef UNREALSDK_IMPORTING

/*
Define this if you're having memory leaks that go through the unreal allocator - which debuggers
normally won't be able to track.
With it defined, all allocations will be stored in a set `unrealsdk::unreal_allocations`.
Since this set uses the default allocator, debuggers should be able to hook onto it to tell you
where the allocations are coming from.
*/
#undef UNREALSDK_UNREAL_ALLOC_TRACKING

using namespace unrealsdk::unreal;

namespace unrealsdk {

namespace {

std::mutex init_mutex{};
std::unique_ptr<game::AbstractHook> hook_instance;

}  // namespace

#ifdef UNREALSDK_UNREAL_ALLOC_TRACKING
std::unordered_set<void*> unreal_allocations{};
#endif

bool init(const std::function<std::unique_ptr<game::AbstractHook>(void)>& game_getter) {
    const std::lock_guard<std::mutex> lock(init_mutex);

    if (hook_instance != nullptr) {
        return false;
    }

    env::load_file();
    logging::init(utils::get_this_dll().parent_path()
                  / env::get(env::LOG_FILE, env::defaults::LOG_FILE));

    auto version = unrealsdk::get_version_string();
    LOG(INFO, "{}", version);
    LOG(INFO, "{}", std::string(version.size(), '='));

    if (MH_Initialize() != MH_OK) {
        throw std::runtime_error("Minhook initialization failed!");
    }

    auto game = game_getter();

    // Initialize the hook before moving it, to weed out any order of initialization problems.
    game->hook();
    hook_instance = std::move(game);
    return true;
}

UNREALSDK_CAPI([[nodiscard]] bool, is_initialized) {
    const std::lock_guard<std::mutex> lock(init_mutex);
    return hook_instance != nullptr;
}

UNREALSDK_CAPI([[nodiscard]] bool, is_console_ready) {
    return hook_instance && hook_instance->is_console_ready();
}

UNREALSDK_CAPI([[nodiscard]] const GObjects*, gobjects) {
    return &hook_instance->gobjects();
}

UNREALSDK_CAPI([[nodiscard]] const GNames*, gnames) {
    return &hook_instance->gnames();
}

UNREALSDK_CAPI(void, fname_init, FName* name, const wchar_t* str, int32_t number) {
    hook_instance->fname_init(name, str, number);
}

UNREALSDK_CAPI(void, fframe_step, FFrame* frame, UObject* obj, void* param) {
    hook_instance->fframe_step(frame, obj, param);
}

UNREALSDK_CAPI(void*, u_malloc, size_t len) {
    auto ptr = hook_instance->u_malloc(len);

#ifdef UNREALSDK_UNREAL_ALLOC_TRACKING
    unreal_allocations.insert(ptr);
#endif

    return ptr;
}
UNREALSDK_CAPI(void*, u_realloc, void* original, size_t len) {
    auto ptr = hook_instance->u_realloc(original, len);

#ifdef UNREALSDK_UNREAL_ALLOC_TRACKING
    unreal_allocations.erase(original);
    unreal_allocations.insert(ptr);
#endif

    return ptr;
}
UNREALSDK_CAPI(void, u_free, void* data) {
#ifdef UNREALSDK_UNREAL_ALLOC_TRACKING
    unreal_allocations.erase(data);
#endif

    hook_instance->u_free(data);
}

UNREALSDK_CAPI(void, process_event, UObject* object, UFunction* function, void* params) {
    hook_instance->process_event(object, function, params);
}

UNREALSDK_CAPI([[nodiscard]] UObject*,
               construct_object,
               UClass* cls,
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

UNREALSDK_CAPI(void, uconsole_output_text, const wchar_t* str, size_t size) {
    // Since we know this will be called a lot before it's ready, even from our own code, do nothing
    // if we don't have a hook yet
    if (hook_instance) {
        hook_instance->uconsole_output_text({str, size});
    }
}

UNREALSDK_CAPI([[nodiscard]] wchar_t*, uobject_path_name, const UObject* obj, size_t& size) {
    auto name = hook_instance->uobject_path_name(obj);
    size = name.size();

    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
    auto mem = reinterpret_cast<wchar_t*>(u_malloc((size + 1) * sizeof(wchar_t)));
    wcsncpy_s(mem, size + 1, name.data(), size);

    return mem;
}

UNREALSDK_CAPI([[nodiscard]] UObject*,
               find_object,
               UClass* cls,
               const wchar_t* name,
               size_t name_size) {
    return hook_instance->find_object(cls, {name, name_size});
}

#ifdef UE4

UNREALSDK_CAPI(void,
               ftext_as_culture_invariant,
               unreal::FText* text,
               unreal::TemporaryFString&& str) {
    hook_instance->ftext_as_culture_invariant(text, std::move(str));
}

#endif

}  // namespace unrealsdk

#endif
