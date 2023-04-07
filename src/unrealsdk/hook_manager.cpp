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

HookMap hooks{};
bool log_all_calls = false;
bool inject_next_call = false;

bool HookList::empty(void) const {
    return this->pre.empty() && this->post.empty();
}

const HookList* preprocess_hook(const std::string& source,
                                const UFunction* func,
                                const UObject* obj) {
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

    auto list = &hooks[func_name];

    // If the list is empty, there are no hooks to run
    // Remove it from the map so that next time the lookup is quicker
    if (list->empty()) {
        hooks.erase(func_name);
        return nullptr;
    }

    return list;
}

bool run_hook_group(const HookGroup& group, HookDetails& hook) {
    // Grab a copy incase the hook removes itself from the group (which would invalidate the
    // iterator)
    auto hook_group_copy = group;

    bool ret = false;
    for (const auto& [_, hook_function] : hook_group_copy) {
        try {
            ret |= hook_function(hook);
        } catch (const std::exception& ex) {
            LOG(ERROR, "An exception occurred during hook processing: {}", ex.what());
        }
    }

    return ret;
}

}  // namespace unrealsdk::hook_manager
