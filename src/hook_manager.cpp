#include "pch.h"

#include "hook_manager.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uobject.h"
#include "unreal/structs/fframe.h"
#include "unreal/wrappers/wrappedargs.h"
#include "unrealsdk.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::hook_manager {

/**
 * @brief Runs all hooks on the given function and decides if it may continue.
 *
 * @param func The function which was called.
 * @param obj The object which called the function.
 * @param args The arguments passed to the function.
 * @param source The source of the hook, used for logging.
 * @return True if to block execution, false if to continue.
 */
static bool run_hooks(UFunction* func,
                      UObject* obj,
                      WrappedArgs& args,
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

bool process_event(UObject* obj, UFunction* func, WrappedArgs& args) {
    return run_hooks(func, obj, args, "ProcessEvent");
}

bool call_function(UObject* obj, FFrame* stack, void* /*result*/, UFunction* func) {
    // NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)

    const auto EXPR_TOKEN_END_FUNCTION_PARMS = 0x16;
    const auto PROPERTY_FLAG_RETURN_PARAM = 0x400;

    auto frame = static_cast<uint8_t*>(calloc(1, func->ParamsSize));

    /*
    for (auto prop = reinterpret_cast<UProperty*>(func->Children);
         *stack->Code != EXPR_TOKEN_END_FUNCTION_PARMS;
         prop = reinterpret_cast<UProperty*>(prop->Next)) {
        bool is_return_param = (prop->PropertyFlags & PROPERTY_FLAG_RETURN_PARAM) != 0;
        if (is_return_param) {
            continue;
        }

        unreal::game->frame_step(stack, stack->Object, frame + prop->Offset_Internal);
    }*/

    WrappedArgs args{func, frame};
    auto block_function = run_hooks(func, obj, args, "CallFunction");

    free(frame);

    if (block_function) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stack->Code++;
    }

    return block_function;

    // NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
}

}  // namespace unrealsdk::hook_manager
