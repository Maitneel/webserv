#include <stdexcept>
#include <iostream>
#include <ostream>
#include <iomanip>
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
        throw InvalidHeader(kContentEncoding);
    }
}

void HTTPRequest::valid_content_length(const std::string &value) {
    size_t front = 0;;
    while (front < value.length() && (value.at(front) == ' ' || value.at(front) == '\t')) {
        front++;
    }
    if (value.length() == front) {
        throw InvalidHeader(kContentLength);
    }
    for (size_t i = front; i < value.length(); i++) {
        if (!is_digit(value.at(i))) {
            throw InvalidHeader(kContentLength);
        }
    }
    try {
        std::cerr << atoi(value.substr(front).c_str()) << std::endl;;
        std::cerr << safe_atoi(value.substr(front)) << std::endl;
        this->content_length = safe_atoi(value.substr(front));
    } catch (std::runtime_error const &) {
        // InvalidHeader ではない気がする //
        throw InvalidHeader(kCONVERT_FAIL);
    }
}

void HTTPRequest::valid_content_type(const std::string &value) {
    size_t front = get_front(value);
    std::string::size_type slash_index = value.find('/');
    std::string::size_type semi_colon_index = value.find(';');
    if (slash_index == std::string::npos) {
        throw InvalidHeader(kContentType);
    }
    try {
        this->content_type.type = value.substr(front, slash_index - 1);
        this->content_type.subtype = value.substr(slash_index + 1, semi_colon_index - slash_index - 1);
    } catch (std::out_of_range const &) {
        throw InvalidHeader(kContentType);
    }
    if (!is_token(this->content_type.type) || !is_token(this->content_type.subtype)) {
        throw InvalidHeader(kContentType);
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
                throw InvalidHeader(kContentType);
            }
            this->content_type.parameter.insert(make_pair(attribute, parameter_value));
        }
    } catch (std::out_of_range const &) {
        throw InvalidHeader(kContentType);
    }
}

void HTTPRequest::valid_date_related_header(const std::string &value, HTTPHeaderExceptType exception_type, std::string *store) {
    size_t front = get_front(value);
    try {
        if (!is_http_date(value.substr(front))) {
            throw InvalidHeader(exception_type);
        }
    } catch (std::out_of_range const &) {
        throw InvalidHeader(exception_type);
    }
    // ここそれっぽい構造体かクラスかなんか作ってそれに詰めた(フォーマットして保存する)方がいい？ //
    *store = value;
}

void HTTPRequest::valid_date(const std::string &value) {
    this->valid_date_related_header(value, kDate, &this->date);
}

void HTTPRequest::valid_expires(const std::string &value) {
    size_t front = get_front(value);
    try {
        if (!is_http_date(value.substr(front))) {
            throw InvalidHeader(kExpries);
        }
    } catch (std::out_of_range const &) {
        throw InvalidHeader(kExpries);
    }
    this->expires = value;
}

void HTTPRequest::valid_form(const std::string &value) {
    // TODO(maitneel)  : 必要があればやります //
    // if (!valid_mailbox(value)) {
    //     throw InvalidHeader(kForm);
    // }
    this->form = value;
}

void HTTPRequest::valid_if_modified_since(const std::string &value) {
    this->valid_date_related_header(value, kIfModifiedSince, &this->if_modified_since);
}

void HTTPRequest::valid_last_modified(const std::string &value) {
    this->valid_date_related_header(value, kLastModified, &this->last_modified);
}

// Pragma           = "Pragma" ":" 1#pragma-directive

// pragma-directive = "no-cache" | extension-pragma
// extension-pragma = token [ "=" word ]
void HTTPRequest::valid_pragma(const std::string &value) {
    size_t front = get_front(value);
    std::vector<std::string> splited_value = split(value.substr(front), ",");
    for (size_t i = 0; i < splited_value.size(); i++) {
        std::string &processing = splited_value.at(i);
        if (processing.length() == 0) {
            continue;
        } else if (processing.at(processing.length() - 1) == ',') {
            processing.erase(processing.end() - 1, processing.end());
        }
        std::cerr << processing << std::endl;
        if (is_crlf(processing.substr(0, 2))) {
            size_t front = 2;
            while (is_sp(processing.at(front)) || is_ht(processing.at(front))) {
                front++;
            }
            processing.erase(processing.begin(), processing.begin() + front);
        }
        if (!is_pragma_directive(processing)) {
            throw InvalidHeader(kPragma);
        }
        this->pragma.push_back(processing);
    }
}

void HTTPRequest::valid_referer(const std::string &value) {
    size_t front = get_front(value);
    std::string trimed_value = value.substr(front);
    is_absolute_uri(trimed_value);
    is_relative_uri(trimed_value);
    if (!is_absolute_uri(trimed_value) && !is_relative_uri(trimed_value)) {
        throw InvalidHeader(kReferer);
    }
    this->referer = trimed_value;
}

static size_t is_opening_parentheses(const std::string &s) {
    size_t result = 0;
    for (size_t i = 0; i < s.length(); i++) {
        if (s.at(i) == '(') {
            result++;
        }
        if (result && s.at(i) == ')') {
            result--;
        }
    }
    return result;
}

void HTTPRequest::valid_user_agent(const std::string &value) {
    size_t front = get_front(value);
    std::vector<std::string> splited_token = split(value.substr(front), " ");
    for (size_t i = 0; i < splited_token.size(); i++) {
        std::string processing = splited_token.at(i);
        while (i + 1 < splited_token.size() && is_opening_parentheses(processing)) {
            processing += splited_token.at(i + 1);
            i++;
        }
        if (processing.at(processing.length() - 1) == ' ') {
            processing.erase(processing.end() - 1, processing.end());
        }
        if (!is_product(processing) && !is_comment(processing)) {
            throw InvalidHeader(kUserAgent);
        }
        this->user_agent.push_back(processing);
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
    validation_func_pair.push_back(std::make_pair("Date", &HTTPRequest::valid_date));
    validation_func_pair.push_back(std::make_pair("Expires", &HTTPRequest::valid_expires));
    validation_func_pair.push_back(std::make_pair("Form", &HTTPRequest::valid_form));
    validation_func_pair.push_back(std::make_pair("Pragma", &HTTPRequest::valid_pragma));
    validation_func_pair.push_back(std::make_pair("Referer", &HTTPRequest::valid_referer));
    validation_func_pair.push_back(std::make_pair("User-Agent", &HTTPRequest::valid_user_agent));


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
            throw InvalidRequest(kRequestLine);
        }
        front++;
        std::string::size_type request_uri_end = buffer.find(' ', front);
        if (request_uri_end == std::string::npos) {
            request_uri_end = buffer.find(crlf, front);
        }
        if (request_uri_end == std::string::npos) {
            throw InvalidRequest(kRequestLine);
        }
        this->request_uri = buffer.substr(front, request_uri_end - front);
        if (!(is_absolute_uri(this->request_uri) || is_abs_path(this->request_uri))) {
            throw InvalidRequest(kRequestLine);
        }
        front = request_uri_end;
        const std::string::size_type crlf_index = buffer.find(crlf, 0);
        if (crlf_index == front) {
            protocol = "HTTP/0.9";
            is_simple_request = true;
        } else {
            if (!is_sp(buffer.at(front))) {
                throw InvalidRequest(kRequestLine);
            }
            front++;
            this->protocol = buffer.substr(front, crlf_index - front);
            if (!is_http_version(this->protocol)) {
                std::cerr << this->protocol << std::endl;
                throw InvalidRequest(kRequestLine);
            }
        }
    } catch (std::out_of_range const &) {
        throw InvalidRequest(kRequestLine);
    }
    crlf_count++;

    for (size_t i = 1; i < splited_buffer.size(); i++) {
        crlf_count++;
        if (is_crlf(splited_buffer[i])) {
            break;
        }
        if (!is_valid_http_header(splited_buffer[i])) {
            throw InvalidRequest(kHTTPHeader);
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
            throw InvalidRequest(kHTTPHeader);
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

// print func
void HTTPRequest::print_info() {
    this->print_info(std::cout);
}

void HTTPRequest::print_info(std::ostream &stream) {
    const size_t width = 25;

    stream << '[' << get_formated_date() << "] " << this->get_method() << ' ' << this->get_request_uri() << ' ' << this->get_protocol() << std::endl;
    stream << "    header : {" << std::endl;
    for (std::map<std::string, std::string>::iterator i = this->header.begin(); i != this->header.end(); i++) {
        stream << "        " << i->first << ": '" << i->second << "'" << std::endl;
    }
    stream << "    }" << std::endl;
    stream << "    body : {" << std::endl;
    stream << "        " << this->entity_body << std::endl;
    stream << "    }" << std::endl;
    stream << std::left << std::setw(width) << "    Allow" << " : [";
    for (size_t i = 0; i < this->allow.size(); i++) {
        stream << '"' << this->allow.at(i) << '"' << ", ";
    }
    stream << "]" << std::endl;
    stream << std::left << std::setw(width) << "    Content-Encoding" << " : " << '"' << this->content_encoding << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Content-length" << " : " << '"' << this->content_length << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Content-Type" << " : " << '"' << this->content_type.type << '/' << this->content_type.subtype << '"' << " parameter : ";
    for (std::multimap<std::string, std::string>::iterator i = this->content_type.parameter.begin(); i != this->content_type.parameter.end(); i++) {
        stream << "{" << i->first << ":" << i->second << "}, ";
    }
    stream << std::endl;
    stream << std::left << std::setw(width) << "    Date" << " : " << '"' << this->date << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Expries" << " : " << '"' << this->expires << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Form" << " : " << '"' << this->form << '"' << std::endl;
    stream << std::left << std::setw(width) << "    If-Modified-Since" << " : " << '"' << this->if_modified_since << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Last-Modified" << " : " << '"' << this->last_modified << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Pragma" << " : [";
    for (size_t i = 0; i < this->pragma.size(); i++) {
        stream << "'" << this->pragma.at(i) << "', ";
    }
    stream << "]" << std::endl;
    stream << std::left << std::setw(width) << "    Referer" << " : " << '"' << this->referer << '"' << std::endl;
    stream << std::left << std::setw(width) << "    User-Agent" << " : " << "[";
    for (size_t i = 0; i < this->user_agent.size(); i++) {
        stream << "'" << this->user_agent.at(i) << "', ";
    }
    stream << "]" << std::endl;
}

// exception class
HTTPRequest::InvalidRequest::InvalidRequest(HTTPRequestExceptType except_type_src) : except_type(except_type_src) {
}

const char *HTTPRequest::InvalidRequest::what() const throw() {
    switch (this->except_type) {
    case kRequestLine:
        return "HTTPRequest: invalid request-line";
        break;
    case kHTTPHeader:
        return "HTTPRequest: invalid HTTP-Header";
        break;
    default:
        return "HTTPRequest: unknow error";
        break;
    }
}

HTTPRequest::InvalidHeader::InvalidHeader(HTTPHeaderExceptType except_type_src) : except_type(except_type_src) {
}

const char *HTTPRequest::InvalidHeader::what() const throw() {
    switch (this->except_type) {
    case kAllow:
        return "HTTPHeader: invalid 'Allow' header";
        break;
    case kContentEncoding:
        return "HTTPHeader: invalid 'Content-Encoding' header";
        break;
    case kContentLength:
        return "HTTPHeader: invalid 'Content-Length' header";
        break;
    case kDate:
        return "HTTPHeader: invalid 'Date' header";
        break;
    case kExpries:
        return "HTTPHeader: invalid 'Expires' header";
        break;
    case kForm:
        return "HTTPHeader: invalid 'Form' header";
        break;
    case kIfModifiedSince:
        return "HTTPHeader: invalid 'If-Modified-Since' header";
        break;
    case kLastModified:
        return "HTTPHeader: invalid 'Last-Modified' header";
        break;
    case kPragma:
        return "HTTPHeader: invalid 'Pragma' header";
        break;
    case kReferer:
        return "HTTPHeader: invalid 'Referer' header";
        break;
    case kCONVERT_FAIL:
        return "HTTPHeader: convert failed";
        break;
    default:
        return "HTTPHeader: unkonw error";
        break;
    }
}

