#ifndef UNREALSDK_UTILS_H
#define UNREALSDK_UTILS_H

#include "unrealsdk/pch.h"

namespace unrealsdk::utils {

/**
 * @brief Narrows a utf-16 wstring to a utf-8 string.
 *
 * @param str The input wstring.
 * @return The output string.
 */
[[nodiscard]] std::string narrow(const std::wstring& wstr);

/**
 * @brief Widens a utf-8 string to a utf-16 wstring.
 *
 * @param str The input string.
 * @return The output wstring.
 */
[[nodiscard]] std::wstring widen(const std::string& str);

/**
 * @brief Get the directory this dll is in.
 * @note Since this is not exported, calls from dlls linking against the shared library return their
 *       own dir, since this function will be linked statically.
 *
 * @return The path of the dll this function is compiled into.
 */
[[nodiscard]] std::filesystem::path get_this_dll_dir(void);

/**
 * @brief Proxy class for an iterator, used to allow multiple range iterators on the same class.
 *
 * @tparam Iterator The iterator type.
 */
template <typename Iterator>
struct IteratorProxy {
    Iterator it_begin;
    Iterator it_end;

   public:
    IteratorProxy(Iterator begin, Iterator end) : it_begin(begin), it_end(end) {}

    /**
     * @brief Gets the internal iterator at it's beginning.
     *
     * @return The iterator
     */
    [[nodiscard]] Iterator begin() const { return this->it_begin; }

    /**
     * @brief Gets the internal iterator at it's end.
     *
     * @return The iterator
     */
    [[nodiscard]] Iterator end() const { return this->it_end; }
};

/**
 * @brief An abstract base class used to pass callbacks across dll boundaries safely.
 * @note You should never have to use this directly, the wrappers should autoconvert for you.
 *
 * @tparam R The return type.
 * @tparam As The argument types.
 */
template <typename R, typename... As>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct AbstractSafeCallback {
    /// The signature functions matching this type should have.
    using Signature = R(As...) UNREALSDK_CAPI_SUFFIX;

    /**
     * @brief Destroys the callback.
     */
    virtual ~AbstractSafeCallback() = default;

    /**
     * @brief Runs the callback.
     *
     * @param args The callback args.
     * @return The return value of the callback
     */
    virtual R operator()(As... args) = 0;
};

/**
 * @brief An implementation of a safe callback.
 * @note This class itself cannot be passed across dlls safely, it may only cross as a pointer to
 *       it's abstract base type.
 *
 * @tparam R The return type.
 * @tparam As The argument types.
 */
template <typename R, typename... A>
struct SafeCallback : AbstractSafeCallback<R, A...> {
    /// The abstract base type this callback inherits from.
    using AbstractBase = AbstractSafeCallback<R, A...>;

   private:
    std::function<typename SafeCallback::Signature> func;

   public:
    /**
     * @brief Constructs a new safe callback from a function.
     *
     * @param func The function to wrap.
     */
    SafeCallback(decltype(func) func) : func(func) {}

    /**
     * @brief Destroys the callback.
     */
    ~SafeCallback() override = default;

    /**
     * @brief Runs the callback.
     *
     * @param args The callback args.
     * @return The return value of the callback
     */
    R operator()(A... args) override {
        if constexpr (std::is_void_v<R>) {
            this->func(args...);
        } else {
            return this->func(args...);
        }
    }
};

}  // namespace unrealsdk::utils

// Custom wstring formatter, which calls narrow
template <>
struct unrealsdk::fmt::formatter<std::wstring> : unrealsdk::fmt::formatter<std::string> {
    auto format(const std::wstring& str, unrealsdk::fmt::format_context& ctx) {
        return formatter<std::string>::format(unrealsdk::utils::narrow(str), ctx);
    }
};

#endif /* UNREALSDK_UTILS_H */
