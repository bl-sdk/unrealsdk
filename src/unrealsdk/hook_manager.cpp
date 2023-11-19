#include "unrealsdk/pch.h"
#include <string_view>

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::hook_manager {

namespace impl {

using Group = utils::StringViewMap<std::wstring, DLLSafeCallback*>;

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
};

}  // namespace impl

namespace {

#ifndef UNREALSDK_IMPORTING
bool should_log_all_calls = false;
bool should_inject_next_call = false;

utils::StringViewMap<std::wstring, impl::List> hooks{};

/**
 * @brief Get the hook group for a certain type from it's list.
 *
 * @param list The hook list to get from.
 * @param type The hook type to get.
 * @return The selected hook group.
 */
impl::Group& get_group_by_type(impl::List& list, Type type) {
    switch (type) {
        case Type::PRE:
            return list.pre;
        case Type::POST:
            return list.post;
        case Type::POST_UNCONDITIONAL:
            return list.post_unconditional;
        default:
            throw std::invalid_argument("Invalid hook type " + std::to_string((uint8_t)type));
    }
}
#endif

}  // namespace

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(void, log_all_calls, bool should_log);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, log_all_calls, bool should_log) {
    should_log_all_calls = should_log;
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
    should_inject_next_call = true;
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
    const std::wstring_view func_view{func, func_size};
    auto iter = hooks.find(func_view);
    if (iter == hooks.end()) {
        iter = hooks.emplace(func_view, impl::List{}).first;
    }

    auto& group = get_group_by_type(iter->second, type);

    const std::wstring_view identifier_view{identifier, identifier_size};
    if (group.contains(identifier_view)) {
        return false;
    }

    group.emplace(identifier_view, callback);
    return true;
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
    const std::wstring_view func_view{func, func_size};
    auto iter = hooks.find(func_view);
    if (iter == hooks.end()) {
        return false;
    }

    auto& group = get_group_by_type(iter->second, type);

    const std::wstring_view identifier_view{identifier, identifier_size};
    return group.contains(identifier_view);
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
    const std::wstring_view func_view{func, func_size};
    auto func_iter = hooks.find(func_view);
    if (func_iter == hooks.end()) {
        return false;
    }

    auto& group = get_group_by_type(func_iter->second, type);

    const std::wstring identifier_view{identifier, identifier_size};
    auto group_iter = group.find(identifier_view);
    if (group_iter == group.end()) {
        return false;
    }

    group_iter->second->destroy();
    group.erase(group_iter);

    /*
    Important Note: While it's tempting, we can't also erase the hook list here if it's empty,
    because we may be being called from inside a hook .

    If we are, we'd end up freeing the list which the native hook still has a reference to, and will
    pass back to us to process further - i.e. it'll use after free, and probably crash.

    Instead, we clean up hook lists during `preprocess_hook`
    */

    return true;
}

#endif

bool remove_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    return UNREALSDK_MANGLE(remove_hook)(func.data(), func.size(), type, identifier.data(),
                                         identifier.size());
}

namespace impl {

#ifndef UNREALSDK_IMPORTING
const List* preprocess_hook(std::string_view source, const UFunction* func, const UObject* obj) {
    if (should_inject_next_call) {
        should_inject_next_call = false;
        return nullptr;
    }

    auto func_name = func->get_path_name();

    if (should_log_all_calls) {
        LOG(MISC, "===== {} called =====", source);
        LOG(MISC, L"Function: {}", func_name);
        LOG(MISC, L"Object: {}", obj->get_path_name());
    }

    if (!hooks.contains(func_name)) {
        return nullptr;
    }

    auto list = &hooks[func_name];

    // Cleanup the list if it's empty
    if (list->empty()) {
        hooks.erase(func_name);
        return nullptr;
    }

    return list;
}

bool has_post_hooks(const List& list) {
    return !list.post.empty() || !list.post_unconditional.empty();
}

bool run_hooks_of_type(const List& list, Type type, Details& hook) {
    // Grab a copy of the revelevant hook group, incase the hook removes itself (which would
    // invalidate the iterator)
    Group group{};

    // Not using `get_group_by_type` because we don't want to throw on an invalid type (and
    // const-ness messes with it).
    switch (type) {
        case Type::PRE:
            group = list.pre;
            break;
        case Type::POST:
            group = list.post;
            break;
        case Type::POST_UNCONDITIONAL:
            group = list.post_unconditional;
            break;
        default:
            LOG(ERROR, "Tried to run hooks of invalid type {}", (uint8_t)type);
            return false;
    }

    bool ret = false;
    for (const auto& [_, hook_function] : group) {
        try {
            ret |= hook_function->operator()(hook);
        } catch (const std::exception& ex) {
            LOG(ERROR, "An exception occurred during hook processing: {}", ex.what());
        }
    }

    return ret;
}
#endif

}  // namespace impl

}  // namespace unrealsdk::hook_manager
