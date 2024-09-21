#ifndef UNREALSDK_UNREAL_WRAPPERS_WRAPPED_MULTICAST_DELEGATE_H
#define UNREALSDK_UNREAL_WRAPPERS_WRAPPED_MULTICAST_DELEGATE_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/structs/tarray.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"

namespace unrealsdk::unreal {

class UFunction;

class WrappedMulticastDelegate {
   public:
    const UFunction* signature;
    UnrealPointer<TArray<FScriptDelegate>> base;

    /**
     * @brief Constructs a new wrapped multicast delegate.
     *
     * @param signature The type of the array elements.
     * @param base The base address of the delegate.
     * @param parent The parent pointer this delegate was retrieved from, used to copy ownership.
     */
    WrappedMulticastDelegate(const UFunction* signature,
                             TArray<FScriptDelegate>* base,
                             const UnrealPointer<void>& parent = {nullptr});

    /**
     * @brief Calls all functions bound to the delegate.
     * @note Multicast delegates are not allowed to have a return value.
     *
     * @tparam Ts The types of the arguments.
     * @param args The arguments.
     * @param params A pre-filled struct of the functions' parameters.
     */
    template <typename... Ts>
    void call(const typename PropTraits<Ts>::Value&... args) const {
        WrappedStruct params{this->signature};
        func_params::write_params<Ts...>(params, args...);
        this->call(params);
    }
    void call(WrappedStruct& params) const;

    /**
     * @brief Adds a new function to this delegate.
     *
     * @param func The function to add.
     */
    void push_back(const BoundFunction& func) const;

    /**
     * @brief Removes all functions bound to this delegate.
     */
    void clear(void) const;
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_WRAPPERS_WRAPPED_MULTICAST_DELEGATE_H */
