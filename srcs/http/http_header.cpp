#include <string>
#include <utility>

#include "get_http_keyword.hpp"

std::pair<std::string, std::string> make_header_pair(std::string src) {
	std::string filed_name = get_first_token(src);
	std::string::size_type filed_value_start = filed_name.length() + 1;
	std::string filed_value = src.substr(filed_value_start, src.length() - filed_value_start - 2);
	return std::make_pair(filed_name, filed_value);
} 