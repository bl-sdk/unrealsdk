#ifndef UNREALSDK_UNREAL_STRUCTS_FSTRING_H
#define UNREALSDK_UNREAL_STRUCTS_FSTRING_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/tarray.h"

namespace unrealsdk::unreal {

/*
We implement three types of FString, for different purposes.

TemporaryFString : Intended to be passed as an input directly into an engine function, and then
                   immediately destroyed. Avoids extra allocations used by the other types, but
                   therefore may not be assigned to or otherwise modified, and is only valid for the
                   lifetime of the string which created it.

UnmanagedFString : An FString assumed to be owned by the engine (i.e. we don't manage it's memory).
                   Allows assignment and conversion back to an stl string, but does not allow copy
                   construction/assignment, which may otherwise leak or use after free.

ManagedFString   : An FString owned by us, which frees memory during the destructor, and therefore
                   allows copy construction/assignment. Inherits from UnmanagedFString.
*/

struct TemporaryFString : public TArray<const wchar_t> {
    /**
     * @brief Construct a new Temporary FString.
     * @warning This object is only valid for the lifespan of the string used to create it.
     *
     * @param str The string this FString should contain.
     */
    TemporaryFString(std::wstring_view str);

    /**
     * @brief Destroys the string.
     */
    ~TemporaryFString() = default;

    // Delete all ways of moving these around, to help discourage keeping references around
    TemporaryFString(const TemporaryFString&) = delete;
    TemporaryFString(TemporaryFString&&) = delete;
    TemporaryFString& operator=(const TemporaryFString&) = delete;
    TemporaryFString& operator=(TemporaryFString&&) = delete;
};

struct UnmanagedFString : public TArray<wchar_t> {
    /**
     * @brief Construct a new unmanaged FString.
     * @note Automatically converts utf8 to utf16 if needed.
     *
     * @param data Pointer to the wchar data.
     * @param count Size of the string.
     * @param max Allocation size of `data`.
     * @param str The stl string to construct this one from.
     * @param other The other FString to construct this one from.
     */
    UnmanagedFString(decltype(data) data = nullptr,
                     decltype(count) count = 0,
                     decltype(max) max = 0);
    UnmanagedFString(std::string_view str);
    UnmanagedFString(std::wstring_view str);
    UnmanagedFString(UnmanagedFString&& other) noexcept;

    /**
     * @brief Destroys the string.
     */
    ~UnmanagedFString() = default;

    /**
     * @brief Assigns to the string.
     * @note Automatically converts utf8 to utf16 if needed.
     *
     * @param str The stl string to assign this one from.
     * @param other The other FString to assign this one from.
     * @return A reference to this FString.
     */
    UnmanagedFString& operator=(std::string_view str);
    UnmanagedFString& operator=(std::wstring_view str);
    UnmanagedFString& operator=(UnmanagedFString&& other) noexcept;

    /**
     * @brief Converts the string to an stl string.
     * @note Automatically converts utf16 to utf8 if needed.
     *
     * @return The converted string.
     */
    operator std::string() const;
    operator std::wstring() const;
    operator std::wstring_view() const;

    // Delete copy construction/assignment since it's easy to leak memory with them.
    UnmanagedFString(const UnmanagedFString& other) = delete;
    UnmanagedFString& operator=(const UnmanagedFString& other) = delete;
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct ManagedFString : public UnmanagedFString {
    /**
     * @brief Construct a new managed FString.
     * @note Automatically converts utf8 to utf16 if needed.
     *
     * @param data Pointer to the wchar data.
     * @param count Size of the string.
     * @param max Allocation size of `data`.
     * @param str The stl string to construct this one from.
     * @param other The other FString to construct this one from.
     */
    using UnmanagedFString::UnmanagedFString;
    ManagedFString(const UnmanagedFString& other);

    /**
     * @brief Assigns to the string.
     * @note Automatically converts utf8 to utf16 if needed.
     *
     * @param str The stl string to assign this one from.
     * @param other The other FString to assign this one from.
     * @return A reference to this FString.
     */
    ManagedFString& operator=(const UnmanagedFString& other);

    /**
     * @brief Destroys the string.
     */
    ~ManagedFString();
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FSTRING_H */
