#ifndef UNREAL_WRAPPERS_BOUND_FUNCTION_H
#define UNREAL_WRAPPERS_BOUND_FUNCTION_H

#include "pch.h"

#include "game/game_hook.h"
#include "unreal/classes/uclass.h"
#include "unreal/classes/ufunction.h"
#include "unreal/classes/uproperty.h"
#include "unreal/wrappers/prop_traits.h"
#include "unreal/wrappers/wrapped_args.h"

namespace unrealsdk::unreal {

class BoundFunction {
   public:
    UFunction* func;
    UObject* object;

   private:
    static constexpr auto PROP_FLAG_PARAM = 0x80;
    static constexpr auto PROP_FLAG_RETURN = 0x400;

    /**
     * @brief Calls this function, given a pointer to it's params struct.
     *
     * @param params A pointer to this function's params struct.
     */
    void call_with_params(void* params) const;

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
    static void set_param(uintptr_t params,
                          UProperty* prop,
                          typename PropTraits<T0>::Value arg0,
                          typename PropTraits<Ts>::Value... args) {
        // Find the next param property
        while (prop != nullptr && (prop->PropertyFlags & PROP_FLAG_PARAM) == 0) {
            prop = prop->PropertyLinkNext;
        }

        if (prop == nullptr) {
            throw std::runtime_error("Too many parameters to function call!");
        }
        if (prop->Class->Name != FName{PropTraits<T0>::CLASS}) {
            throw std::invalid_argument("Property was of invalid type "
                                        + (std::string)prop->Class->Name);
        }
        if (prop->ArrayDim > 1) {
            throw std::runtime_error(
                "Function has static array argument - unsure how to handle, aborting!");
        }

        set_property<T0>(reinterpret_cast<T0*>(prop), 0, params, arg0);

        auto next = prop->PropertyLinkNext;
        if constexpr (sizeof...(Ts) > 0) {
            set_param<Ts...>(params, next, args...);
        } else {
            validate_no_more_params(next);
        }
    }

    /**
     * @brief Gets the return value of a completed function call.
     *
     * @tparam R The return type.
     * @param params A pointer to the params struct.
     * @return The function's return value.
     */
    template <typename R>
    typename PropTraits<R>::Value get_return_value(uintptr_t params) {
        UProperty* prop = this->func->PropertyLink;
        while (prop != nullptr) {
            if ((prop->PropertyFlags & PROP_FLAG_RETURN) != 0) {
                if (prop->ArrayDim > 1) {
                    throw std::runtime_error(
                        "Function has static array return param - unsure how to handle, aborting!");
                }
                return get_property<R>(reinterpret_cast<R*>(prop), 0, params);
            }
            prop = prop->PropertyLinkNext;
        }
        throw std::runtime_error("Couldn't find return param!");
    }

   public:
    /**
     * @brief Calls this function.
     *
     * @tparam R The return type. May be void.
     * @tparam Ts The types of the arguments.
     * @param args The arguments.
     * @return The function's return value.
     */
    template <typename R, typename... Ts>
    typename PropTraits<R>::Value call(typename PropTraits<Ts>::Value... args) {
        auto params = game::malloc(this->func->get_struct_size());

        UProperty* base_prop = this->func->PropertyLink;
        if constexpr (sizeof...(Ts) > 0) {
            set_param<Ts...>(reinterpret_cast<uintptr_t>(params), base_prop, args...);
        } else {
            validate_no_more_params(base_prop);
        }

        this->call_with_params(params);

        if constexpr (std::is_void_v<R>) {
            game::free(params);
        } else {
            // TODO: use after free when the return is a reference type (structs, arrays)
            auto ret = this->get_return_value<R>(reinterpret_cast<uintptr_t>(params));
            game::free(params);
            return ret;
        }
    }
    template <typename R>
    typename PropTraits<R>::Value call(const WrappedArgs& args) {
        if (args.type != this->func) {
            throw std::runtime_error(
                "Tried to call a function using wrapped args of a different function.");
        }

        auto size = this->func->get_struct_size();
        auto params = game::malloc(size);
        memcpy(params, args.base, size);

        this->call_with_params(params);

        if constexpr (std::is_void_v<R>) {
            game::free(params);
        } else {
            // TODO: use after free when the return is a reference type (structs, arrays)
            auto ret = this->get_return_value<R>(reinterpret_cast<uintptr_t>(params));
            game::free(params);
            return ret;
        }
    }
};

template <>
struct PropTraits<UFunction> {
    using Value = BoundFunction;
    static inline const wchar_t* const CLASS = L"Function";
};

template <>
struct PropTraits<void> {
    using Value = void;
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_BOUND_FUNCTION_H */
