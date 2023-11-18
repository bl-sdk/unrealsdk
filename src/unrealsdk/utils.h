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
[[nodiscard]] std::string narrow(std::wstring_view wstr);

/**
 * @brief Widens a utf-8 string to a utf-16 wstring.
 *
 * @param str The input string.
 * @return The output wstring.
 */
[[nodiscard]] std::wstring widen(std::string_view str);

/**
 * @brief Get the directory this dll is in.
 * @note This function is linked statically, calls from dlls linking against the shared library will
 *       return their own path.
 *
 * @return The path of the dll this function is compiled into.
 */
[[nodiscard]] std::filesystem::path get_this_dll(void);

/**
 * @brief Get the main executable we're running within.
 *
 * @return The path of the main executable.
 */
[[nodiscard]] std::filesystem::path get_executable(void);

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
 * @brief A class used to safely pass callbacks, which may be lambdas or other complex callables,
 *        across dll boundaries safely.
 * @note You should never have to use this directly, the wrappers should convert everything for you.
 *
 * @warning When using this type to implement a wrapper, it must be handled with great care. It
 *          relies on two dangerous rules to work properly:
 *          - It must never be stored or passed by value, only ever by pointer.
 *          - Before destroying the pointer, you must manually call the `destroy` method.
 *
 * @tparam R The return type. May be void.
 * @tparam As The argument types.
 */
template <typename R, typename... As>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct DLLSafeCallback {
    /// The inner type of the function this callback runs.
    using InnerFunc = std::function<R(As...) noexcept(false)>;

   private:
    // We can't trust an actual virtual function table to have a consistent layout -  e.g. clang
    // uses two destructors (freeing and non-freeing), MSVC only uses one (with a bool arg)
    // Instead, create our own manually.

    // Note MSVC relies on `noexcept(false)` to allow exceptions to cross dll boundaries.
    struct PseudoVFTable {
        void (*destroy)(DLLSafeCallback* self) noexcept(false);
        R (*call)(DLLSafeCallback* self, As... args) noexcept(false);
    };

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    static void destroy(DLLSafeCallback* self) noexcept(false) { delete self; }
    static R call(DLLSafeCallback* self, As... args) noexcept(false) { return self->func(args...); }

    static const constexpr PseudoVFTable DEFAULT_VFTABLE = {
        &DLLSafeCallback::destroy,
        &DLLSafeCallback::call,
    };
    // Use volatile to prevent optimization from calling our copies of the function, it has to go
    // through the vftable
    volatile const PseudoVFTable* vftable = &DEFAULT_VFTABLE;

    InnerFunc func;

   public:
    /**
     * @brief Constructs a new safe callback from a function.
     *
     * @param func The function to wrap.
     */
    DLLSafeCallback(InnerFunc func) : func(func) {}

    /**
     * @brief Destroys the callback.
     */
    void destroy(void) { this->vftable->destroy(this); }

    /**
     * @brief Runs the callback.
     *
     * @param args The callback args.
     * @return The return value of the callback
     */
    R operator()(As... args) { return this->vftable->call(this, args...); }
};

}  // namespace unrealsdk::utils

// Custom wstring formatter, which calls narrow
template <>
struct unrealsdk::fmt::formatter<std::wstring> : unrealsdk::fmt::formatter<std::string> {
    auto format(const std::wstring& str, unrealsdk::fmt::format_context& ctx) const {
        return formatter<std::string>::format(unrealsdk::utils::narrow(str), ctx);
    }
};

#endif /* UNREALSDK_UTILS_H */
