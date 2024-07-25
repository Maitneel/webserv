#ifndef _INCLUDE_EXTEND_STDLIB_HPP_
#define _INCLUDE_EXTEND_STDLIB_HPP_

#include <vector>
#include <string>

std::vector<std::string> split(const std::string &src, const std::string &delimiter);
std::vector<std::string> escaped_quote_split(const std::string &src, const std::string &delimiter);

#endif
