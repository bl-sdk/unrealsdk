#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include "pch.h"

namespace unrealsdk::unreal {

class FFrame;
class UFunction;
class UObject;
class WrappedArgs;

}  // namespace unrealsdk::unreal

namespace unrealsdk::hook_manager {

using callback = bool(unreal::UFunction*, unreal::UObject*, unreal::WrappedArgs&);
using func_name = std::wstring;
using identifier = std::wstring;
using map = std::unordered_map<func_name, std::unordered_map<identifier, std::function<callback>>>;

extern map hooks;
extern bool log_all_calls;
extern bool inject_next_call;

/**
 * @brief Handles a `UObject::ProcessEvent` call, processes any hooks, and decides if to block
 *        execution.
 *
 * @param obj The object which called the function.
 * @param func The function which was called.
 * @param args The arguments passed to the function.
 * @return True if to block execution, false if to continue.
 */
bool process_event(unreal::UObject* obj, unreal::UFunction* func, unreal::WrappedArgs& args);

/**
 * @brief Handles a `UObject::CallFunction` call, processes any hooks, and decides if to block
 *        execution.
 *
 * @param obj The object which called the function.
 * @param stack The current stack frame.
 * @param result Pointer to the result variable.
 * @param func The function which was called.
 * @return True if to block execution, false if to continue.
 */
bool call_function(unreal::UObject* obj, unreal::FFrame* stack, void* result, unreal::UFunction* func);

}  // namespace unrealsdk::hook_manager

#endif /* HOOK_MANAGER_H */
