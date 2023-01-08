#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include "pch.h"

namespace unrealsdk::unreal {

class UObject;
class UFunction;
struct FStruct;

}  // namespace unrealsdk::unreal

namespace unrealsdk::hook_manager {

using callback = bool(const unreal::UFunction*, const unreal::UObject*, unreal::FStruct*);
using func_name = std::string;
using identifier = std::string;
using map = std::unordered_map<func_name, std::unordered_map<identifier, std::function<callback>>>;

/**
 * @brief Processes all hook functions in the map, and decides if to block execution.
 *
 * @param source The source of the hook, used for logging.
 * @param func The function which was called.
 * @param obj The object which called the function.
 * @param params The parameters passed to the function.
 * @return True if to block execution, false if to continue.
 */
bool process_hooks(const std::string& source,
                   const unreal::UFunction* func,
                   const unreal::UObject* obj,
                   unreal::FStruct* params);

}  // namespace unrealsdk::hook_manager

#endif /* HOOK_MANAGER_H */
