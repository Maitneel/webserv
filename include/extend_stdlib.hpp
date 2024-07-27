#ifndef INCLUDE_EXTEND_STDLIB_HPP_
#define INCLUDE_EXTEND_STDLIB_HPP_

#include <vector>
#include <string>

std::vector<std::string> split(const std::string &src, const std::string &delimiter);
std::vector<std::string> escaped_quote_split(const std::string &src, const std::string &delimiter);
int safe_atoi(std::string str);

#endif  // INCLUDE_EXTEND_STDLIB_HPP_
