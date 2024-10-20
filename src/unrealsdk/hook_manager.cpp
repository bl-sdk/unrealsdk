#include "unrealsdk/pch.h"

#include "unrealsdk/env.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

using namespace unrealsdk::unreal;

#pragma region Implementation
#ifndef UNREALSDK_IMPORTING
namespace unrealsdk::hook_manager::impl {

namespace {

/*
We store all hooks in a multi layered mapping.

The outermost level is the FName of the function, which lets us do a very quick compare to discard
most calls. The second level is the full path name string, what was passed in, which we use to
confirm we've got the right hook.

`preprocess_hook` looks through these first two levels to determine if the called function has any
hooks, and returns the "List" of all hooks which apply to that function at any stage.

The list then maps each hook type into "Group"s. Each group is a full set to hooks to run on a
specific stage.
*/

using Group = utils::StringViewMap<std::wstring, DLLSafeCallback*>;

}  // namespace

struct List {
    Group pre;
    Group post;
    Group post_unconditional;

    /**
     * @brief Checks if all groups in the list are empty.
     *
     * @return True if all groups are empty
     */
    [[nodiscard]] bool empty(void) const {
        return this->pre.empty() && this->post.empty() && this->post_unconditional.empty();
    }

    /**
     * @brief Gets a hook group on this list from it's type.
     *
     * @param type The hook type to get.
     * @return A reference to the selected hook group on this object, or nullptr if calling the safe
     *         version and the type's invalid.
     */
    [[nodiscard]] Group& get_group_by_type(Type type) {
        switch (type) {
            case Type::PRE:
                return this->pre;
            case Type::POST:
                return this->post;
            case Type::POST_UNCONDITIONAL:
                return this->post_unconditional;
            default:
                throw std::invalid_argument("Invalid hook type " + std::to_string((uint8_t)type));
        }
    }
    // We only need the const version in practice
    [[nodiscard]] const Group* get_safe_group_by_type(Type type) const noexcept {
        switch (type) {
            case Type::PRE:
                return &this->pre;
            case Type::POST:
                return &this->post;
            case Type::POST_UNCONDITIONAL:
                return &this->post_unconditional;
            default:
                return nullptr;
        }
    }
};

namespace {

thread_local bool should_inject_next_call = false;

bool should_log_all_calls = false;
std::unique_ptr<std::wostream> log_all_calls_stream;
std::mutex log_all_calls_stream_mutex{};

std::unordered_map<FName, utils::StringViewMap<std::wstring, List>> hooks{};

void log_all_calls(bool should_log) {
    // Only keep this file stream open while we need it
    if (should_log) {
        const std::lock_guard<std::mutex> lock(log_all_calls_stream_mutex);
        log_all_calls_stream = std::make_unique<std::wofstream>(
            utils::get_this_dll().parent_path()
                / env::get(env::LOG_ALL_CALLS_FILE, env::defaults::LOG_ALL_CALLS_FILE),
            std::ofstream::trunc);
    }

    should_log_all_calls = should_log;

    if (!should_log) {
        const std::lock_guard<std::mutex> lock(log_all_calls_stream_mutex);
        log_all_calls_stream = nullptr;
    }
}

void inject_next_call(void) {
    should_inject_next_call = true;
}

/**
 * @brief Extracts the object name from a hook function's full path name.
 *
 * @param func The full function path name.
 * @return The FName we expect it's object to use.
 */
FName extract_func_obj_name(std::wstring_view func) {
    auto idx = func.find_last_of(L".:");
    if (idx == std::wstring_view::npos) {
        return FName{std::wstring{func}};
    }
    return FName{std::wstring{func.substr(idx + 1)}};
}

bool add_hook(std::wstring_view func,
              Type type,
              std::wstring_view identifier,
              DLLSafeCallback* callback) {
    auto fname = extract_func_obj_name(func);

    auto& path_name_map = hooks[fname];

    // Doing this a bit weirdly to try avoid allocating a new string - we can get via string view,
    // but setting requires converting to a full string first
    auto iter = path_name_map.find(func);
    if (iter == path_name_map.end()) {
        iter = path_name_map.emplace(func, List{}).first;
    }

    auto& group = iter->second.get_group_by_type(type);
    if (group.contains(identifier)) {
        return false;
    }

    group.emplace(identifier, callback);
    return true;
}

bool has_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    auto fname = extract_func_obj_name(func);

    auto fname_iter = hooks.find(fname);
    if (fname_iter == hooks.end()) {
        return false;
    }
    auto& path_name_map = fname_iter->second;

    auto path_name_iter = path_name_map.find(func);
    if (path_name_iter == path_name_map.end()) {
        return false;
    }

    return path_name_iter->second.get_group_by_type(type).contains(identifier);
}

bool remove_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    auto fname = extract_func_obj_name(func);

    auto fname_iter = hooks.find(fname);
    if (fname_iter == hooks.end()) {
        return false;
    }
    auto& path_name_map = fname_iter->second;

    auto path_name_iter = path_name_map.find(func);
    if (path_name_iter == path_name_map.end()) {
        return false;
    }

    auto& group = path_name_iter->second.get_group_by_type(type);
    auto group_iter = group.find(identifier);
    if (group_iter == group.end()) {
        return false;
    }

    group_iter->second->destroy();
    group.erase(group_iter);

    /*
    Important Note: While it's tempting, we can't also erase the higher levels here if they're
    empty, because we may be being called from inside a hook, since this may cause a use after free.

    We make sure to take a copy of the group in `run_hooks_of_type`, so invalidating iterators
    within it is not a concern.

    Instead, we clean up during `preprocess_hook`
    */

    return true;
}

}  // namespace

const List* preprocess_hook(std::wstring_view source, const UFunction* func, const UObject* obj) {
    if (should_inject_next_call) {
        should_inject_next_call = false;
        return nullptr;
    }

    // Want to delay filling this, but if we're logging all calls we need it straight away
    std::wstring func_name{};

    if (should_log_all_calls) {
        func_name = func->get_path_name();
        auto obj_name = obj->get_path_name();

        const std::lock_guard<std::mutex> lock(log_all_calls_stream_mutex);
        *log_all_calls_stream << source << L'\t' << func_name << L'\t' << obj_name << L'\n';
    }

    // Check if anything matches the function FName
    auto fname_iter = hooks.find(func->Name);
    if (fname_iter == hooks.end()) {
        return nullptr;
    }
    auto& path_name_map = fname_iter->second;
    if (path_name_map.empty()) {
        hooks.erase(func->Name);
        return nullptr;
    }

    // Now check the full path name
    if (!should_log_all_calls) {
        func_name = func->get_path_name();
    }
    auto path_name_iter = path_name_map.find(func_name);
    if (path_name_iter == path_name_map.end()) {
        return nullptr;
    }
    auto& list = path_name_iter->second;
    if (list.empty()) {
        path_name_map.erase(func_name);
        return nullptr;
    }

    return &list;
}

bool has_post_hooks(const List& list) {
    return !list.post.empty() || !list.post_unconditional.empty();
}

bool run_hooks_of_type(const List& list, Type type, Details& hook) {
    const Group* group_ptr = list.get_safe_group_by_type(type);
    if (group_ptr == nullptr) {
        LOG(ERROR, "Tried to run hooks of invalid type {}", (uint8_t)type);
        return false;
    }

    // Grab a copy of the revelevant hook group, in case the hook removes itself (which would
    // invalidate the iterator)
    const Group group = *group_ptr;

    bool ret = false;
    for (const auto& [_, hook_function] : group) {
        try {
            ret |= hook_function->operator()(hook);
        } catch (const std::exception& ex) {
            LOG(ERROR, "An exception occurred during hook processing");
            LOG(ERROR, L"Function: {}", hook.func.func->get_path_name());
            LOG(ERROR, "Exception: {}", ex.what());
        }
    }

    return ret;
}

}  // namespace unrealsdk::hook_manager::impl
#endif
#pragma endregion

// =================================================================================================

#pragma region Public Interface
namespace unrealsdk::hook_manager {

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(void, log_all_calls, bool should_log);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, log_all_calls, bool should_log) {
    impl::log_all_calls(should_log);
}
#endif
void log_all_calls(bool should_log) {
    UNREALSDK_MANGLE(log_all_calls)(should_log);
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(void, inject_next_call);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, inject_next_call) {
    impl::inject_next_call();
}
#endif
void inject_next_call(void) {
    UNREALSDK_MANGLE(inject_next_call)();
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool,
               add_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size,
               DLLSafeCallback* callback);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool,
               add_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size,
               DLLSafeCallback* callback) {
    return impl::add_hook({func, func_size}, type, {identifier, identifier_size}, callback);
}
#endif

bool add_hook(std::wstring_view func,
              Type type,
              std::wstring_view identifier,
              const Callback& callback) {
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    return UNREALSDK_MANGLE(add_hook)(func.data(), func.size(), type, identifier.data(),
                                      identifier.size(), new DLLSafeCallback(callback));
    // NOLINTEND(cppcoreguidelines-owning-memory)
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool,
               has_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool,
               has_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size) {
    return impl::has_hook({func, func_size}, type, {identifier, identifier_size});
}
#endif

bool has_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    return UNREALSDK_MANGLE(has_hook)(func.data(), func.size(), type, identifier.data(),
                                      identifier.size());
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool,
               remove_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size);
#endif
#ifndef UNREALSDK_IMPORTING

UNREALSDK_CAPI(bool,
               remove_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size) {
    return impl::remove_hook({func, func_size}, type, {identifier, identifier_size});
}

#endif

bool remove_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    return UNREALSDK_MANGLE(remove_hook)(func.data(), func.size(), type, identifier.data(),
                                         identifier.size());
}

}  // namespace unrealsdk::hook_manager

#pragma endregion
