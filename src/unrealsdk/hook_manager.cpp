#include "unrealsdk/pch.h"

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::hook_manager {

#ifndef UNREALSDK_IMPORTING
bool log_all_calls = false;
bool inject_next_call = false;
#endif

namespace impl {

using Group = std::unordered_map<std::wstring, std::function<Callback>>;

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
std::unordered_map<std::wstring, impl::List> hooks{};

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
UNREALSDK_CAPI bool add_hook(const wchar_t* func,
                             size_t func_size,
                             Type type,
                             const wchar_t* identifier,
                             size_t identifier_size,
                             Callback* callback);
#endif
#ifdef UNREALSDK_IMPORTING
bool add_hook(const std::wstring& func,
              Type type,
              const std::wstring& identifier,
              Callback* callback) {
    return add_hook(func.c_str(), func.size(), type, identifier.c_str(), identifier.size(),
                    callback);
}
#else
bool add_hook(const std::wstring& func,
              Type type,
              const std::wstring& identifier,
              Callback* callback) {
    auto& group = get_group_by_type(hooks[func], type);
    if (group.contains(identifier)) {
        return false;
    }
    group[identifier] = callback;
    return true;
}
#endif
#ifdef UNREALSDK_EXPORTING
bool add_hook(const wchar_t* func,
                             size_t func_size,
                             Type type,
                             const wchar_t* identifier,
                             size_t identifier_size,
                             Callback* callback) {
    return add_hook({func, func_size}, type, {identifier, identifier_size}, callback);
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI bool has_hook(const wchar_t* func,
                             size_t func_size,
                             Type type,
                             const wchar_t* identifier,
                             size_t identifier_size);
#endif
#ifdef UNREALSDK_IMPORTING
bool has_hook(const std::wstring& func, Type type, const std::wstring& identifier) {
    return has_hook(func.c_str(), func.size(), type, identifier.c_str(), identifier.size());
}
#else
bool has_hook(const std::wstring& func, Type type, const std::wstring& identifier) {
    if (!hooks.contains(func)) {
        return false;
    }
    return get_group_by_type(hooks[func], type).contains(identifier);
}
#endif
#ifdef UNREALSDK_EXPORTING
bool has_hook(const wchar_t* func,
                             size_t func_size,
                             Type type,
                             const wchar_t* identifier,
                             size_t identifier_size) {
    return has_hook({func, func_size}, type, {identifier, identifier_size});
}
#endif

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI bool remove_hook(const wchar_t* func,
                                size_t func_size,
                                Type type,
                                const wchar_t* identifier,
                                size_t identifier_size);
#endif
#ifdef UNREALSDK_IMPORTING
bool remove_hook(const std::wstring& func, Type type, const std::wstring& identifier) {
    return remove_hook(func.c_str(), func.size(), type, identifier.c_str(), identifier.size());
}
#else
bool remove_hook(const std::wstring& func, Type type, const std::wstring& identifier) {
    if (!hooks.contains(func)) {
        return false;
    }
    auto& group = get_group_by_type(hooks[func], type);
    if (!group.contains(identifier)) {
        return false;
    }
    group.erase(identifier);
    if (group.empty()) {
        hooks.erase(func);
    }
    return true;
}
#endif
#ifdef UNREALSDK_EXPORTING
bool remove_hook(const wchar_t* func,
                                size_t func_size,
                                Type type,
                                const wchar_t* identifier,
                                size_t identifier_size) {
    return remove_hook({func, func_size}, type, {identifier, identifier_size});
}
#endif

namespace impl {

#ifndef UNREALSDK_IMPORTING
const List* preprocess_hook(const std::string& source, const UFunction* func, const UObject* obj) {
    if (inject_next_call) {
        inject_next_call = false;
        return nullptr;
    }

    auto func_name = func->get_path_name();

    if (log_all_calls) {
        LOG(MISC, "===== {} called =====", source);
        LOG(MISC, L"Function: {}", func_name);
        LOG(MISC, L"Object: {}", obj->get_path_name());
    }

    if (!hooks.contains(func_name)) {
        return nullptr;
    }

    return &hooks[func_name];
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
            ret |= hook_function(hook);
        } catch (const std::exception& ex) {
            LOG(ERROR, "An exception occurred during hook processing: {}", ex.what());
        }
    }

    return ret;
}
#endif

}  // namespace impl

}  // namespace unrealsdk::hook_manager
