#ifndef HTTP_HEADER_HPP
# define HTTP_HEADER_HPP

#include <utility>
#include <string>

std::pair<std::string, std::string> make_header_pair(std::string src);

#endif