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
 * @note This is used to address the fact that std::function may differ between STL implementations.
 * @note You should never have to use this directly, the wrappers should convert everything for you.
 *
 * @tparam F The std::function type this callback is based on.
 * @tparam R The return type. May be void.
 * @tparam As The argument types.
 */
template <typename F>
struct DLLSafeCallback;
template <typename R, typename... As>
struct DLLSafeCallback<std::function<R(As...)>> {
   private:
    /*
    This Inner type is what does most of the work. It has some very non-standard semantics, so the
    outer DLLSafeCallback just safely wraps a pointer around it.

    On constructing a new callback, we'll construct a new Inner struct in the current DLL - using
    the current std::function layout and current allocator. We then rely on "virtual" functions
    whenever we interact with it, to make sure it's always code from the original DLL. This is
    critical because the inner struct's layout can be completely different between implementations,
    we can't even rely on it being a consistent size.

    One complication is we can't trust an actual virtual function table to have a consistent layout.
    For example, clang uses two destructors (freeing and non-freeing), while MSVC only uses one
    (with a bool arg). We also need to make sure we avoid devirtualization, especially with LTO. To
    handle these, we have to create our own vftable manually.
    */
    struct Inner {
       public:
        struct PseudoVFTable {
            // MSVC relies on `noexcept(false)` to allow exceptions to cross dll boundaries.
            void (*destroy)(Inner* self) noexcept(false);
            R (*call)(Inner* self, As... args) noexcept(false);
        };

       private:
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        static void destroy(Inner* self) noexcept(false) { delete self; }
        static R call(Inner* self, As... args) noexcept(false) {
            return self->func(std::forward<As>(args)...);
        }

        static const constexpr PseudoVFTable DEFAULT_VFTABLE = {
            &Inner::destroy,
            &Inner::call,
        };

       public:
        // Volatile means assume can be changed externally - e.g. because it was assigned by a
        // different dll. Otherwise optimization might try devirtualize it.
        volatile const PseudoVFTable* vftable = &DEFAULT_VFTABLE;

       private:
        // Must be after the vftable, since we can't rely on it's size.
        std::function<R(As...)> func;

       public:
        Inner(std::function<R(As...)> func) : func(func) {}
    };

    Inner* inner;

   public:
    /**
     * @brief Constructs a new dll safe callback.
     *
     * @param func The function to wrap.
     */
    DLLSafeCallback(std::function<R(As...)> func) : inner(new Inner(std::move(func))) {}
    DLLSafeCallback(DLLSafeCallback&& other) noexcept
        : inner(std::exchange(other.inner, nullptr)) {}

    /**
     * @brief Assigns to the dll safe callback.
     *
     * @param other The other callback to assign from.
     * @return A reference to this callback.
     */
    DLLSafeCallback& operator=(DLLSafeCallback&& other) noexcept {
        std::swap(this->inner, other.inner);
    }

    /**
     * @brief Destroys the dll safe callback.
     */
    ~DLLSafeCallback() {
        if (this->inner != nullptr) {
            this->inner->vftable->destroy(this->inner);
            // The original dll has safely destroyed the inner struct, so we're free to "leak" it
            this->inner = nullptr;
        }
    }

    // No copy construction/assignment. Haven't really needed it yet, and would require implementing
    // some form of reference counting.
    DLLSafeCallback(const DLLSafeCallback&) = delete;
    DLLSafeCallback& operator=(const DLLSafeCallback& other) = delete;

    /**
     * @brief Runs the callback.
     *
     * @param args The callback args.
     * @return The return value of the callback
     */
    R operator()(As... args) {
        if (this->inner == nullptr) {
            throw std::runtime_error("tried to run a null callback!");
        }
        return this->inner->vftable->call(this->inner, std::forward<As>(args)...);
    }
};

/**
 * @brief RAII class which suspends all other threads for it's lifespan.
 */
class ThreadSuspender {
   public:
    ThreadSuspender(void);
    ~ThreadSuspender();

    ThreadSuspender(const ThreadSuspender&) = delete;
    ThreadSuspender(ThreadSuspender&&) = delete;
    ThreadSuspender& operator=(const ThreadSuspender&) = delete;
    ThreadSuspender& operator=(ThreadSuspender&&) = delete;
};

namespace {

template <typename T>
struct StringViewHash {
    using is_transparent = void;

    [[nodiscard]] size_t operator()(const T& str) const { return std::hash<T>{}(str); }
    [[nodiscard]] size_t operator()(
        std::basic_string_view<typename T::value_type, typename T::traits_type> str) const {
        return std::hash<std::basic_string_view<typename T::value_type, typename T::traits_type>>{}(
            str);
    }
};

}  // namespace

/**
 * @brief A map where the key is a string, which may also be compared using a string view.
 *
 * @tparam Key The key string type to use.
 * @tparam T The value type.
 */
template <typename Key, typename T, typename Allocator = std::allocator<std::pair<const Key, T>>>
using StringViewMap = std::unordered_map<Key, T, StringViewHash<Key>, std::equal_to<>, Allocator>;

}  // namespace unrealsdk::utils

// Custom wstring formatter, which calls narrow
template <>
struct std::formatter<std::wstring> : std::formatter<std::string> {
    auto format(const std::wstring& str, std::format_context& ctx) const {
        return formatter<std::string>::format(unrealsdk::utils::narrow(str), ctx);
    }
};

#endif /* UNREALSDK_UTILS_H */
