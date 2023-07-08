#ifndef UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_H
#define UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

namespace impl {

class UnrealPointerControl;

}  // namespace impl

class UStruct;

/**
 * @brief A smart pointer to a block of unreal-allocated memory.
 * @note Safe to cross dll boundaries.
 *
 * @tparam T the wrapped pointer type. May be void.
 */
template <typename T>
class UnrealPointer {
   private:
    template <typename U>
    friend class UnrealPointer;

    // The control pointer may be null if not owned by the sdk - i.e. a pointer to a struct in the
    // middle of an objet.
    impl::UnrealPointerControl* control;
    T* ptr;

    /**
     * @brief Releases the reference this pointer is holding onto, turning it into a null pointer.
     */
    void release(void);

    /**
     * @brief Tries to increment the reference count in the control block, and turns this pointer
     *        into a null pointer if an exception is thrown.
     */
    void attach(void) {
        // If inc_ref throws, we can't be sure if our reference was actually added.
        // If we tried releasing our reference, but it wasn't actually added, we'd free too early,
        // better to do nothing
        try {
            if (this->control != nullptr) {
                this->control->inc_ref();
            }
        } catch (...) {
            this->control = nullptr;
            this->ptr = nullptr;
            throw;
        }
    }

   public:
    /**
     * @brief Constructs a null pointer.
     */
    UnrealPointer(std::nullptr_t) : control(nullptr), ptr(nullptr) {}

    /**
     * @brief Constructs a pointer to a new block of memory holding a specific struct.
     *
     * @param struct_type The type of struct to construct.
     */
    UnrealPointer(const UStruct* struct_type);

    /**
     * @brief Construct a new pointer pointing at memory owned by another pointer.
     *
     * @param owner The pointer which owns the memory.
     * @param ptr The address to point at.
     */
    template <typename U>
    UnrealPointer(const UnrealPointer<U>& owner, T* ptr) : control(owner.control), ptr(ptr) {
        this->attach();
    }

    /**
     * @brief Constructs a new pointer.
     *
     * @param other The other pointer to copy/move from.
     */
    UnrealPointer(const UnrealPointer<T>& other) : control(other.control), ptr(other.ptr) {
        this->attach();
    }
    UnrealPointer(UnrealPointer<T>&& other) noexcept
        : control(std::exchange(other.control, nullptr)), ptr(std::exchange(other.ptr, nullptr)) {}

    /**
     * @brief Assigns to the pointer
     *
     * @param other The other pointer to copy/move from/
     * @return A reference to this pointer.
     */
    UnrealPointer<T>& operator=(const UnrealPointer<T>& other) {
        this->release();
        *this = UnrealPointer<T>{other};
        return *this;
    }
    UnrealPointer<T>& operator=(UnrealPointer<T>&& other) noexcept {
        std::swap(this->control, other.control);
        std::swap(this->ptr, other.ptr);
        return *this;
    }

    /**
     * @brief Destroys this pointer.
     */
    ~UnrealPointer(void) { this->release(); }

    /**
     * @brief Implicitly decays into a void pointer.
     *
     * @return A void version of this pointer.
     */
    operator UnrealPointer<void>() const { return UnrealPointer<void>{*this, this->ptr}; }

    /**
     * @brief Gets the address this pointer points to
     *
     * @return The address the pointer points to.
     */
    operator T*() const { return this->ptr; }
    [[nodiscard]] T* get(void) const noexcept { return this->ptr; }

    /**
     * @brief Deferences this pointer.
     *
     * @return The value the pointer points to
     */
    template <typename U = T,
              typename = std::enable_if_t<std::is_same_v<U, T> && std::negation_v<std::is_void<U>>>>
    U& operator*() const noexcept {
        return *this->ptr;
    }
    T* operator->() const noexcept { return this->ptr; }
};

namespace impl {

class UnrealPointerControl {
   private:
    // As an implementation detail, we don't need to store the base address of the allocation
    // because we put the control block at the start, our address *is* the base address
    std::atomic<size_t> refs;

    // Strictly speaking, std::atomic is not guaranteed to be safe to cross dll boundaries
    // However in practice, we expect it to be implemented entirely in hardware
    // To make sure it's safe, we:
    // - Make sure it's always lock free - i.e. it's implemented in hardware.
    // - Make all accesses go through virtual functions, so we always use the implementation of
    //   the library which created the atomic. This kind of does the same as the last point, but
    //   it's an extra level of safety.
    // - Make sure it has the same size and alignment as the base type, to make sure it won't
    //   change the overall layout of the control block.
    static_assert(std::atomic<size_t>::is_always_lock_free
                      && sizeof(std::atomic<size_t>) == sizeof(size_t)
                      && alignof(std::atomic<size_t>) == alignof(size_t),
                  "atomic size_t may not be safe to cross dll boundaries");

   public:
    // The only way get a pointer owned by the sdk (excluding calling u_malloc) is by making a new
    // struct - we have a specific constructor on the just for it.
    // Structs need to run custom deleter. Because all sdk-owned pointers are structs, the only
    // deleter metadata we need is the struct type, we can get away with just storing that.
    // If we add we add more ways to allocate new memory, this will have to turn into a more
    // comprehensive custom deleter.
    const UStruct* deleter_struct;

    /**
     * @brief Construct a new control block.
     */
    UnrealPointerControl(const UStruct* deleter_struct) : refs(0), deleter_struct(deleter_struct) {}

    /**
     * @brief Destroys the control block.
     *
     */
    virtual ~UnrealPointerControl(void) = default;

    /**
     * @brief Increments the reference count.
     *
     * @return The new reference count.
     */
    virtual size_t inc_ref(void);

    /**
     * @brief Decrements the reference count.
     *
     * @return The new reference count.
     */
    virtual size_t dec_ref(void);

    UnrealPointerControl(const UnrealPointerControl& other) = delete;
    UnrealPointerControl(UnrealPointerControl&& other) noexcept = delete;
    UnrealPointerControl& operator=(const UnrealPointerControl& other) = delete;
    UnrealPointerControl& operator=(UnrealPointerControl&& other) noexcept = delete;
};

}  // namespace impl

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_WRAPPERS_UNREAL_POINTER_H */
