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

}

#endif /* UTILS_H */
