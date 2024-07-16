#include <string>
#include "keywords.hpp"

std::string get_first_token(std::string s) {
	size_t token_length;
	for (token_length = 0; token_length < s.length(); token_length++) {
		if (!is_token_element(s.at(token_length))) {
			break;
		}
	}
	return s.substr(0, token_length);
	
}