#ifndef UNREALSDK_LOCKS_H
#define UNREALSDK_LOCKS_H

#ifndef UNREALSDK_IMPORTING

namespace unrealsdk::locks {

/**
 * @brief RAII class to hold the function call lock.
 * @note Noop if locking function calls are not enabled.
 */
struct FunctionCall {
   public:
    FunctionCall();
    ~FunctionCall();

    FunctionCall(const FunctionCall&) = delete;
    FunctionCall(FunctionCall&&) noexcept = delete;
    FunctionCall& operator=(const FunctionCall&) = delete;
    FunctionCall& operator=(FunctionCall&&) noexcept = delete;

    /**
     * @brief Checks if the function call lock is enabled.
     *
     * @return True if enabled, false if disabled.
     */
    static bool enabled(void);
};

}  // namespace unrealsdk::locks

#endif

#endif /* UNREALSDK_LOCKS_H */
