#include "pch.h"

#include "hook_manager.h"
#include "unreal/classes/ufunction.h"
#include "unreal/structs/fstruct.h"
#include "unrealsdk.h"

namespace unrealsdk::hook_manager {

void add(const func_name& name,
         const identifier& identifier,
         const std::function<callback>& callback) {
    unrealsdk::hooks[name][identifier] = callback;
}

bool process_hooks(const std::string& source,
                   const unreal::UFunction* func,
                   const unreal::UObject* obj,
                   unreal::FStruct* params) {
    auto function_name = func->get_path_name<char>();
    if (unrealsdk::log_all_calls) {
        LOG(HOOKS, "===== %s called =====\nFunction Name: %s\nCaller Name: %s", source.c_str(),
            function_name.c_str(), obj->get_path_name<char>().c_str());
    }

    bool block = false;
    for (const auto& hook_pair : unrealsdk::hooks[function_name]) {
        auto hook_function = hook_pair.second;
        if (hook_function(func, obj, params)) {
            block = true;
            // Deliberately don't break, so that other hook functions get a chance to run too
        }
    }

    return block;
}

}  // namespace unrealsdk::hook_manager
