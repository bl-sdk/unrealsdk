#include "pch.h"

#include "hook_manager.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/wrapped_struct.h"
#include "unrealsdk.h"
#include "utils.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::hook_manager {

map hooks{};
bool log_all_calls = false;
bool inject_next_call = false;

const list* preprocess_hook(const std::string& source, const UFunction* func, const UObject* obj) {
    if (inject_next_call) {
        inject_next_call = false;
        return nullptr;
    }

    auto func_name = func->get_path_name();

    if (log_all_calls) {
        LOG(MISC, "===== {} called =====", source);
        LOG(MISC, "Function: {}", func_name);
        LOG(MISC, "Object: {}", obj->get_path_name());
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

bool process_hook(const list& list, HookDetails& hook) {
    // Grab a copy incase the hook removes itself from the list (which would invalidate the
    // iterator)
    auto hook_list_copy = list;

    bool block = false;
    for (const auto& [_, hook_function] : hook_list_copy) {
        try {
            if (hook_function(hook)) {
                block = true;

                // Deliberately don't break, so that other hook functions get a chance to run too
                // Functions will still need to negotiate with each other if they both want to
                // overwrite the return value, we can't cleanly handle that
            }
        } catch (const std::exception& ex) {
            LOG(ERROR, "An exception occured during hook processing: {}", ex.what());
        }
    }

    return block;
}

}  // namespace unrealsdk::hook_manager
