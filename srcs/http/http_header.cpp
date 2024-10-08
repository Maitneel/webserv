#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

#include <iostream>

#include "http_request.hpp"

#include "get_http_keyword.hpp"
#include "http_validation.hpp"
#include "extend_stdlib.hpp"

std::pair<std::string, std::string> make_header_pair(std::string src) {
    std::string filed_name, filed_value;
    try {
        filed_name = get_first_token(src);
        to_lower(&filed_name);
        std::string::size_type filed_value_start = filed_name.length() + 1;
        if (src.at(filed_name.length()) != ':') {
            // TODO(someone): resopnse with 400 status code
            // https://www.rfc-editor.org/rfc/rfc9112.html#section-5.1-2
            throw HTTPRequest::InvalidRequest(kHTTPHeader);
        }
        filed_value = src.substr(filed_value_start, src.length() - filed_value_start - 2);
    } catch (...) {
        // TODO(someone): resopnse with 400 status code
        throw HTTPRequest::InvalidRequest(kHTTPHeader);
    }
    return std::make_pair(filed_name, filed_value);
}

std::vector<std::string> convert_allow_to_vector(std::string src) {
    src.erase(remove(src.begin(), src.end(), ' '), src.end());
    src.erase(remove(src.begin(), src.end(), '\t'), src.end());

    std::vector<std::string> allow = split(src, ",");
    for (size_t i = 0; i < allow.size(); i++) {
        if (allow.at(i).at(allow.at(i).length() - 1) == ',') {
            allow.at(i).erase(allow.at(i).length() - 1, 1);
        }
        for (size_t j = 0; j < allow.at(i).length(); j++) {
            if (!is_token_element(allow.at(i).at(j))) {
                throw HTTPRequest::InvalidHeader(kAllow);
            }
        }
    }
    return allow;
}
