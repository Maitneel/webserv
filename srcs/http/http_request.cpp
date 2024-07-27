#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <utility>

#include "http_request.hpp"
#include "http_validation.hpp"
#include "get_http_keyword.hpp"
#include "http_header.hpp"
#include "extend_stdlib.hpp"


size_t get_front(const std::string &str) {
    size_t result = 0;
    while (result < str.length() && (str.at(result) == ' ' || str.at(result) == '\t')) {
        result++;
    }
    return result;
}

void HTTPRequest::valid_allow(const std::string &value) {
    try {
        this->allow = convert_allow_to_vector(value);
    } catch (InvalidHeader &e) {
        // TODO(maitneel)  : 後で考える
    }
}

void HTTPRequest::valid_authorization(const std::string &value) {
    // TODO(maitneel)
}

void HTTPRequest::valid_content_encoding(const std::string &value) {
    size_t front = get_front(value);
    this->content_encoding = value.substr(front);
    if (!is_token(this->content_encoding)) {
        throw InvalidHeader(CONTENT_ENCODING);
    }
}

void HTTPRequest::valid_content_length(const std::string &value) {
    size_t front = 0;;
    while (front < value.length() && (value.at(front) == ' ' || value.at(front) == '\t')) {
        front++;
    }
    if (value.length() == front) {
        throw InvalidHeader(CONTENT_LENGTH);
    }
    for (size_t i = front; i < value.length(); i++) {
        if (!is_digit(value.at(i))) {
            throw InvalidHeader(CONTENT_LENGTH);
        }
    }
    try {
        std::cerr << atoi(value.substr(front).c_str()) << std::endl;;
        std::cerr << safe_atoi(value.substr(front)) << std::endl;
        this->content_length = safe_atoi(value.substr(front));
    } catch (std::runtime_error) {
        // InvalidHeader ではない気がする //
        throw InvalidHeader(CONVERT_FAIL);
    }
}

void HTTPRequest::valid_content_type(const std::string &value) {
    size_t front = get_front(value);
    std::string::size_type slash_index = value.find('/');
    std::string::size_type semi_colon_index = value.find(';');
    if (slash_index == std::string::npos) {
        throw InvalidHeader(CONTENT_TYPE);
    }
    try {
        this->content_type.type = value.substr(front, slash_index - 1);
        this->content_type.subtype = value.substr(slash_index + 1, semi_colon_index - slash_index - 1);
    } catch (std::out_of_range) {
        throw InvalidHeader(CONTENT_TYPE);
    }
    if (!is_token(this->content_type.type) || !is_token(this->content_type.subtype)) {
        throw InvalidHeader(CONTENT_TYPE);
    }
    if (semi_colon_index == std::string::npos) {
        return;
    }
    try {
        std::vector<std::string> splited_parameter = escaped_quote_split(value.substr(semi_colon_index + 1), ";");
        for (size_t i = 0; i < splited_parameter.size(); i++) {
            std::cerr << splited_parameter.at(i) << std::endl;
            if (splited_parameter.at(i) == ";" || splited_parameter.at(i) == "") {
                continue;
            }
            size_t equal_index = splited_parameter.at(i).find('=');
            std::string attribute = splited_parameter.at(i).substr(0, equal_index);
            std::string parameter_value = splited_parameter.at(i).substr(equal_index + 1);
            if (parameter_value.at(parameter_value.length() - 1) == ';') {
                parameter_value.erase(parameter_value.end() - 1, parameter_value.end());
            }
            if (!is_token(attribute) || (!is_token(parameter_value) && !is_quoted_string(parameter_value))) {
                throw InvalidHeader(CONTENT_TYPE);
            }
            this->content_type.parameter.insert(make_pair(attribute, parameter_value));
        }
    } catch (std::out_of_range) {
        throw InvalidHeader(CONTENT_TYPE);
    }
}

HTTPRequest::HTTPRequest(const int fd) {
    // TODO(maitneel):
}

HTTPRequest::HTTPRequest(std::string buffer) : is_simple_request(false), header(), entity_body(), allow(), content_encoding(), content_length() {
    // こいつらなんかいい感じに初期化しリストとかで初期化したい(やり方がわからなかった)　//
    validation_func_pair.push_back(std::make_pair("Allow", &HTTPRequest::valid_allow));
    validation_func_pair.push_back(std::make_pair("Authorization", &HTTPRequest::valid_authorization));
    validation_func_pair.push_back(std::make_pair("Content-Encoding", &HTTPRequest::valid_content_encoding));
    validation_func_pair.push_back(std::make_pair("Content-Length", &HTTPRequest::valid_content_length));
    validation_func_pair.push_back(std::make_pair("Content-Type", &HTTPRequest::valid_content_type));

    std::string crlf;
    crlf += CR;
    crlf += LF;
    std::vector<std::string> splited_buffer = escaped_quote_split(buffer, crlf);
    size_t crlf_count = 0;
    // ここあとでかきなおす //
    try {
        size_t front = 0;

        this->method = get_first_token(buffer);
        front += this->method.length();
        if (!is_sp(buffer[front])) {
            throw InvalidRequest(REQUEST_LINE);
        }
        front++;
        std::string::size_type request_uri_end = buffer.find(' ', front);
        if (request_uri_end == std::string::npos) {
            request_uri_end = buffer.find(crlf, front);
        }
        if (request_uri_end == std::string::npos) {
            throw InvalidRequest(REQUEST_LINE);
        }
        this->request_uri = buffer.substr(front, request_uri_end - front);
        if (!(is_absolute_uri(this->request_uri) || is_abs_path(this->request_uri))) {
            throw InvalidRequest(REQUEST_LINE);
        }
        front = request_uri_end;
        const std::string::size_type crlf_index = buffer.find(crlf, 0);
        if (crlf_index == front) {
            protocol = "HTTP/0.9";
            is_simple_request = true;
        } else {
            if (!is_sp(buffer.at(front))) {
                throw InvalidRequest(REQUEST_LINE);
            }
            front++;
            this->protocol = buffer.substr(front, crlf_index - front);
            if (!is_http_version(this->protocol)) {
                std::cerr << this->protocol << std::endl;
                throw InvalidRequest(REQUEST_LINE);
            }
        }
    } catch (const std::out_of_range e) {
        throw InvalidRequest(REQUEST_LINE);
    }
    crlf_count++;

    for (size_t i = 1; i < splited_buffer.size(); i++) {
        crlf_count++;
        if (is_crlf(splited_buffer[i])) {
            break;
        }
        if (!is_valid_http_header(splited_buffer[i])) {
            throw InvalidRequest(HTTP_HEADER);
        }
        this->header.insert(make_header_pair(splited_buffer[i]));
    }

    // この後のヘッダーの処理 RFC1945 の例だとコロンの後にスペースが入ってるけどこれ消していいのかわかんねぇ //
    for (size_t i = 0; i < HTTPRequest::validation_func_pair.size(); i++) {
        std::pair<std::string, void(HTTPRequest::*)(const std::string &)> target = this->validation_func_pair.at(i);
        if (this->header.find(target.first) != this->header.end()) {
            (this->*(target.second))(this->header.at(target.first));
        }
    }

    if (crlf_count < splited_buffer.size()) {
        if (this->header.find("Content-Length") == this->header.end()) {
            throw InvalidRequest(HTTP_HEADER);
        }
        try {
            this->entity_body = splited_buffer[crlf_count].substr(0, this->content_length);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            // throw いんたーなるさーばーえらー的なやつ //
        }
    }
}

HTTPRequest::~HTTPRequest() {
}


// getter //
const std::string &HTTPRequest::get_method() const {
    return this->method;
}

const std::string &HTTPRequest::get_request_uri() const {
    return this->request_uri;
}

const std::string &HTTPRequest::get_protocol() const {
    return this->protocol;
}


// exception class
HTTPRequest::InvalidRequest::InvalidRequest(t_http_request_except_type except_type_src) : except_type(except_type_src) {
}

const char *HTTPRequest::InvalidRequest::what() const throw() {
    switch (this->except_type) {
    case REQUEST_LINE:
        return "HTTPRequest: invalid request-line";
        break;
    case HTTP_HEADER:
        return "HTTPRequest: invalid HTTP-Header";
        break;
    default:
        break;
    }
}

HTTPRequest::InvalidHeader::InvalidHeader(t_http_header_except_type except_type_src) : except_type(except_type_src) {
}

const char *HTTPRequest::InvalidHeader::what() const throw() {
    switch (this->except_type) {
    case ALLOW:
        return "HTTPHeader: invalid 'Allow' header";
        break;
    case CONTENT_ENCODING:
        return "HTTPHeader: invalid 'Content-Encoding' header";
        break;
    case CONTENT_LENGTH:
        return "HTTPHeader: invalid 'Content-Length' header";
        break;
    }
}

