#ifndef INCLUDE_HTTP_HEADER_HPP_
#define INCLUDE_HTTP_HEADER_HPP_

#include <utility>
#include <string>

std::pair<std::string, std::string> make_header_pair(std::string src);
std::vector<std::string> convert_allow_to_vector(std::string src);

#endif  // INCLUDE_HTTP_HEADER_HPP_
