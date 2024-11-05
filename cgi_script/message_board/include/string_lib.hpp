#ifndef INCLUDE_STRING_LIB_HPP_
#define INCLUDE_STRING_LIB_HPP_

#include <string>
#include <vector>

std::string gen_random_str(const size_t length = 8);
std::vector<std::string> escaped_quote_split(const std::string &src, const std::string &delimiter);
std::string trim(const std::string &src, const std::string &remove_char);

#endif // INCLUDE_STRING_LIB_HPP_