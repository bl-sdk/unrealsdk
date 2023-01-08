#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include "pch.h"

namespace unrealsdk::unreal {

class UObject;
class UFunction;
class WrappedArgs;

}  // namespace unrealsdk::unreal

namespace unrealsdk::hook_manager {

using callback = bool(const unreal::UFunction*, unreal::UObject*, unreal::WrappedArgs&);
// TODO: replace with UFunction directly
using func_name = std::string;
using identifier = std::string;
using map = std::unordered_map<func_name, std::unordered_map<identifier, std::function<callback>>>;

/**
 * @brief Processes all hook functions in the map, and decides if to block execution.
 *
 * @param func The function which was called.
 * @param obj The object which called the function.
 * @param args The arguments passed to the function.
 * @param source The source of the hook, used for logging.
 * @return True if to block execution, false if to continue.
 */
bool process_hooks(const unreal::UFunction* func,
                   unreal::UObject* obj,
                   unreal::WrappedArgs& args,
                   const std::string& source);

}  // namespace unrealsdk::hook_manager

#endif /* HOOK_MANAGER_H */
