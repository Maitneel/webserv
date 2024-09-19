#include <string>
#include "http_validation.hpp"

std::string get_first_token(std::string s) {
    size_t token_length;
    for (token_length = 0; token_length < s.length(); token_length++) {
        if (!is_token_element(s.at(token_length))) {
            break;
        }
    }
    return s.substr(0, token_length);
}

std::string get_first_ows(const std::string &str, const size_t &start) {
    std::string result;
    for (size_t i = start; i < str.length(); i++) {
        if (str.at(i) == SP || str.at(i) == HTAB) {
            result.push_back(str.at(i));
        } else {
            break;
        }
    }
    return result;
}

std::string get_first_ows(const std::string &str) {
    return get_first_ows(str, 0);
}
