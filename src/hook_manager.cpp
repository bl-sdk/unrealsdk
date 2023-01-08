#include "pch.h"

#include "hook_manager.h"
#include "unreal/classes/ufunction.h"
#include "unreal/wrappers/wrappedargs.h"
#include "unrealsdk.h"

namespace unrealsdk::hook_manager {

bool process_hooks(const unreal::UFunction* func,
                   unreal::UObject* obj,
                   unreal::WrappedArgs& args,
                   const std::string& source) {
    auto function_name = func->get_path_name<char>();
    if (unrealsdk::log_all_calls) {
        LOG(HOOKS, "===== %s called =====", source.c_str());
        LOG(HOOKS, "Function Name: %s", function_name.c_str());
        LOG(HOOKS, "Object Name: %s", obj->get_path_name<char>().c_str());
    }

    if (unrealsdk::hooks.count(function_name) == 0) {
        return false;
    }

    bool block = false;
    for (const auto& hook_pair : unrealsdk::hooks[function_name]) {
        auto hook_function = hook_pair.second;
        if (hook_function(func, obj, args)) {
            block = true;
            // Deliberately don't break, so that other hook functions get a chance to run too
        }
    }

    return block;
}

}  // namespace unrealsdk::hook_manager
