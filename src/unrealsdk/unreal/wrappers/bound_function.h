#ifndef UNREALSDK_UNREAL_WRAPPERS_BOUND_FUNCTION_H
#define UNREALSDK_UNREAL_WRAPPERS_BOUND_FUNCTION_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

struct FName;

class BoundFunction {
   public:
    UFunction* func;
    UObject* object;

   private:
    /**
     * @brief Calls this function, given a pointer to it's params struct.
     *
     * @param params A pointer to this function's params struct.
     */
    void call_with_params(void* params) const;

    /**
     * @brief Get the next parameter property in the chain.
     * @note Includes optional properties.
     *
     * @param prop The current property to start at.
     * @return The next parameter, or nullptr on reaching the end of the chain.
     */
    [[nodiscard]] static UProperty* get_next_param(UProperty* prop);

    /**
     * @brief Checks that there are no more required params for a function call.
     *
     * @param prop The next unparsed parameter property object.
     */
    static void validate_no_more_params(UProperty* prop);

    /**
     * @brief Tail recursive function to set all args in a function's params struct.
     *
     * @tparam T0 The type of the first arg, which this call will set.
     * @tparam Ts The types of the remaining args.
     * @param params A pointer to the params struct.
     * @param prop The next unparsed parameter property object.
     * @param arg0 This argument.
     * @param args The remaining arguments.
     */
    template <typename T0, typename... Ts>
    static void set_param(WrappedStruct& params,
                          UProperty* prop,
                          const typename PropTraits<T0>::Value& arg0,
                          const typename PropTraits<Ts>::Value&... args) {
        if (prop == nullptr) {
            throw std::runtime_error("Too many parameters to function call!");
        }
        if (prop->ArrayDim > 1) {
            throw std::runtime_error(
                "Function has static array argument - unsure how to handle, aborting!");
        }

        params.set<T0>(validate_type<T0>(prop), 0, arg0);

        auto next = get_next_param(prop);
        if constexpr (sizeof...(Ts) > 0) {
            set_param<Ts...>(params, next, args...);
        } else {
            validate_no_more_params(next);
        }
    }

    /**
     * @brief Write all arguments into a function's params struct.
     *
     * @tparam Ts The types of the arguments.
     * @param params The params struct to write to. Modified in place.
     * @param args The arguments.
     */
    template <typename... Ts>
    static void write_params(WrappedStruct& params, const typename PropTraits<Ts>::Value&... args) {
        UProperty* prop = params.type->PropertyLink;
        if (prop != nullptr && (prop->PropertyFlags & UProperty::PROP_FLAG_PARAM) == 0) {
            prop = get_next_param(prop);
        }

        if constexpr (sizeof...(Ts) > 0) {
            set_param<Ts...>(params, prop, args...);
        } else {
            validate_no_more_params(prop);
        }
    }

    template <typename R>
    using call_return_type =
        std::conditional_t<std::is_void_v<R>, void, typename PropTraits<R>::Value>;

    /**
     * @brief Gets the return value of a completed function call.
     *
     * @tparam R The return type.
     * @param params A pointer to the params struct.
     * @return The function's return value.
     */
    template <typename R>
    call_return_type<R> get_return_value(const WrappedStruct& params) {
        if constexpr (std::is_void_v<R>) {
            // Do nothing
        } else {
            auto ret = this->func->find_return_param();
            if (ret == nullptr) {
                throw std::runtime_error("Couldn't find return param!");
            }
            if (ret->ArrayDim > 1) {
                throw std::runtime_error(
                    "Function has static array return param - unsure how to handle, aborting!");
            }

            return get_property<R>(validate_type<R>(ret), 0,
                                   reinterpret_cast<uintptr_t>(params.base.get()), params.base);
        }
    }

   public:
    /**
     * @brief Calls this function.
     *
     * @tparam R The return type. If `void`, the return value is ignored (even if it exists). If
     *           `WrappedStruct`, returns the params struct after the call (useful for out params).
     * @tparam Ts The types of the arguments.
     * @param args The arguments.
     * @param params A pre-filled struct of the function's parameters.
     * @return The function's return value.
     */
    template <typename R, typename... Ts>
    call_return_type<R> call(const typename PropTraits<Ts>::Value&... args) {
        WrappedStruct params{this->func};
        write_params<Ts...>(params, args...);

        this->call_with_params(params.base.get());
        if constexpr (!std::is_void_v<R>) {
            return this->get_return_value<R>(params);
        }
    }
    template <typename R>
    call_return_type<R> call(WrappedStruct& params) {
        if (params.type != this->func) {
            throw std::runtime_error(
                "Tried to call function with pre-filled parameters of incorrect type: "
                + (std::string)params.type->Name);
        }

        this->call_with_params(params.base.get());
        if constexpr (!std::is_void_v<R>) {
            return this->get_return_value<R>(params);
        }
    }
};

// UFunction isn't a property, so we don't define a prop traits class, we don't want the default
// getters/setters to work, we don't want to be able to pass it as an arg to a function, etc.

// Instead, we explicitly instantiate `UObject->get<UFunction>` as the only getter.

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_WRAPPERS_BOUND_FUNCTION_H */
