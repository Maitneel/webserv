#ifndef _INCLUDE_HTTP_HEADER_HPP_
# define _INCLUDE_HTTP_HEADER_HPP_

#include <utility>
#include <string>

std::pair<std::string, std::string> make_header_pair(std::string src);

#endif
