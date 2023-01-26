#ifndef UTILS_H
#define UTILS_H

namespace unrealsdk::utils {

/**
 * @brief Narrows a utf-16 wstring to a utf-8 string.
 *
 * @param str The input wstring.
 * @return The output string.
 */
std::string narrow(const std::wstring& wstr);

/**
 * @brief Widens a utf-8 string to a utf-16 wstring.
 *
 * @param str The input string.
 * @return The output wstring.
 */
std::wstring widen(const std::string& str);

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
    Iterator begin() { return this->it_begin; }

    /**
     * @brief Gets the internal iterator at it's end.
     *
     * @return The iterator
     */
    Iterator end() { return this->it_end; }
};

}  // namespace unrealsdk::utils

#endif /* UTILS_H */
