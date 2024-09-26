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
        this->allow_ = convert_allow_to_vector(value);
    } catch (InvalidHeader &e) {
        // TODO(maitneel)  : 後で考える
    }
}

void HTTPRequest::valid_authorization(const std::string &value) {
    // TODO(maitneel)
}

void HTTPRequest::valid_content_encoding(const std::string &value) {
    size_t front = get_front(value);
    this->content_encoding_ = value.substr(front);
    if (!is_token(this->content_encoding_)) {
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
        this->content_length_ = safe_atoi(value.substr(front));
    } catch (std::runtime_error const &) {
        // InvalidHeader ではない気がする //
        throw InvalidHeader(kConvertFail);
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
        this->content_type_.type = value.substr(front, slash_index);
        this->content_type_.subtype = value.substr(slash_index + 1, semi_colon_index - slash_index - 1);
    } catch (std::out_of_range const &) {
        throw InvalidHeader(kContentType);
    }
    if (!is_token(this->content_type_.type) || !is_token(this->content_type_.subtype)) {
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
            this->content_type_.parameter.insert(make_pair(attribute, parameter_value));
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
    this->valid_date_related_header(value, kDate, &this->date_);
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
    this->expires_ = value;
}

void HTTPRequest::valid_form(const std::string &value) {
    // TODO(maitneel)  : 必要があればやります //
    // if (!valid_mailbox(value)) {
    //     throw InvalidHeader(kForm);
    // }
    this->form_ = value;
}

void HTTPRequest::valid_if_modified_since(const std::string &value) {
    this->valid_date_related_header(value, kIfModifiedSince, &this->if_modified_since_);
}

void HTTPRequest::valid_last_modified(const std::string &value) {
    this->valid_date_related_header(value, kLastModified, &this->last_modified_);
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
        this->pragma_.push_back(processing);
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
    this->referer_ = trimed_value;
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
        this->user_agent_.push_back(processing);
    }
}

HTTPRequest::HTTPRequest(const int fd) {
    // TODO(maitneel):
}

void HTTPRequest::add_valid_funcs() {
    // こいつらなんかいい感じに初期化しリストとかで初期化したい(やり方がわからなかった)　//
    if (this->protocol == http_0_9 || this->protocol == HTTP_1_0) {
        validation_func_pair.push_back(std::make_pair("allow", &HTTPRequest::valid_allow));
        validation_func_pair.push_back(std::make_pair("authorization", &HTTPRequest::valid_authorization));
        validation_func_pair.push_back(std::make_pair("content-encoding", &HTTPRequest::valid_content_encoding));
        validation_func_pair.push_back(std::make_pair("content-length", &HTTPRequest::valid_content_length));
        validation_func_pair.push_back(std::make_pair("content-type", &HTTPRequest::valid_content_type));
        validation_func_pair.push_back(std::make_pair("date", &HTTPRequest::valid_date));
        validation_func_pair.push_back(std::make_pair("expires", &HTTPRequest::valid_expires));
        validation_func_pair.push_back(std::make_pair("form", &HTTPRequest::valid_form));
        validation_func_pair.push_back(std::make_pair("pragma", &HTTPRequest::valid_pragma));
        validation_func_pair.push_back(std::make_pair("referer", &HTTPRequest::valid_referer));
        validation_func_pair.push_back(std::make_pair("user-agent", &HTTPRequest::valid_user_agent));
    } else if (this->protocol == HTTP_1_1) {
        validation_func_pair.push_back(std::make_pair("content-length", &HTTPRequest::valid_content_length));
        validation_func_pair.push_back(std::make_pair("content-type", &HTTPRequest::valid_content_type));
    }
}

std::string HTTPRequest::parse_method(const std::string &request_line) {
    const std::string::size_type first_sp_index = request_line.find(' ');

    const std::string method = request_line.substr(0, first_sp_index);
    if (!is_token(method)) {
        throw InvalidRequest(kRequestLine);
    }
    return method;
}

std::string HTTPRequest::parse_request_uri(const std::string &request_line) {
    const std::string::size_type first_sp_index = request_line.find(' ');
    const std::string::size_type second_sp_index = request_line.find(' ', first_sp_index + 1);
    const std::string::size_type crlf_index = request_line.find(CRLF);
    std::string::size_type substr_length = 0;

    if (second_sp_index != std::string::npos) {
        substr_length = second_sp_index - first_sp_index;
    } else {
        substr_length = crlf_index - first_sp_index;
    }
    const std::string request_uri = request_line.substr(first_sp_index + 1, substr_length - 1);
    if (!(is_absolute_uri(request_uri) || is_abs_path(request_uri))) {
        throw InvalidRequest(kRequestLine);
    }
    return request_uri;
}

std::string HTTPRequest::parse_protocol(const std::string &request_line) {
    const std::string::size_type first_sp_index = request_line.find(' ');
    const std::string::size_type second_sp_index = request_line.find(' ', first_sp_index + 1);
    const std::string::size_type crlf_index = request_line.find(CRLF);

    if (second_sp_index == std::string::npos) {
        return "HTTP/0.9";
    }
    std::string protocol = request_line.substr(second_sp_index + 1, crlf_index - second_sp_index - 1);
    if (!is_http_version(protocol)) {
        throw InvalidRequest(kRequestLine);
    }
    return protocol;
}

void HTTPRequest::parse_request_line(const std::string &request_line) {
    try {
        this->method = this->parse_method(request_line);
        this->request_uri = this->parse_request_uri(request_line);
        this->protocol = this->parse_protocol(request_line);
    } catch (...) {
        throw InvalidRequest(kRequestLine);
    }
}

size_t HTTPRequest::registor_field(const std::vector<std::string> &splited_buffer) {
    size_t registor_count = 0;
    for (size_t i = 1; i < splited_buffer.size(); i++) {
        if (is_crlf(splited_buffer[i])) {
            registor_count++;
            break;
        }
        if (!is_valid_http_header(splited_buffer[i])) {
            throw InvalidRequest(kHTTPHeader);
        }
        std::pair<std::string, std::string> header_pair = make_header_pair(splited_buffer[i]);
        if (this->protocol == HTTP_1_1) {
            // https://www.rfc-editor.org/rfc/rfc9110.html#name-field-values
            // >> a recipient of CR, LF, or NUL within a field value MUST either reject the message or replace each of those characters with SP before further processing or forwarding of that message. Field values containing other CTL characters are also invalid;
            // この reject って400系のresponseを返せってことなのか、filedを無視しろってことなのかどっち? //
            header_pair.second = trim_string(header_pair.second, " ");
        }
        std::map<std::string, std::vector<std::string> >::iterator it = this->header_.find(header_pair.first);
        if (it == this->header_.end()) {
            this->header_.insert(make_pair(header_pair.first, std::vector<std::string>(1, header_pair.second)));
        } else {
            it->second.push_back(header_pair.second);
        }
        registor_count++;
    }
    this->transform_headers();
    return registor_count;
}

void HTTPRequest::valid_headers() {
    this->add_valid_funcs();

    for (size_t i = 0; i < HTTPRequest::validation_func_pair.size(); i++) {
        std::pair<std::string, void(HTTPRequest::*)(const std::string &)> target = this->validation_func_pair.at(i);
        std::map<std::string, std::vector<std::string> >::iterator it = this->header_.find(target.first);
        if (it != this->header_.end()) {
            for (size_t i = 0; i < it->second.size(); i++) {
                (this->*(target.second))(it->second[i]);
            }
        }
    }
}

void HTTPRequest::transform_content_type() {
    std::map<std::string, std::vector<std::string> >::iterator content_type_local_val = this->header_.find("content-type");
    for (size_t i = 0; i < content_type_local_val->second.size(); i++) {
        to_lower(&content_type_local_val->second[i]);
    }
}

void HTTPRequest::transform_headers() {
    if (this->header_.find("content-type") != this->header_.end()) {
        transform_content_type();
    }
}

void HTTPRequest::registor_entity_body(const std::vector<std::string> &splited_buffer, const size_t front) {
    // この後のヘッダーの処理 RFC1945 の例だとコロンの後にスペースが入ってるけどこれ消していいのかわかんねぇ //
    if (front < splited_buffer.size()) {
        if (this->header_.find("content-length") == this->header_.end()) {
            throw InvalidRequest(kHTTPHeader);
        }
        try {
            this->entity_body_ = splited_buffer[front].substr(0, this->content_length_);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            // throw いんたーなるさーばーえらー的なやつ //
        }
    }
    this->transform_headers();
}

HTTPRequest::HTTPRequest(std::string buffer) : is_simple_request(false), header_(), entity_body_(), allow_(), content_encoding_(), content_length_() {
    std::vector<std::string> splited_buffer = escaped_quote_split(buffer, CRLF);
    this->parse_request_line(splited_buffer[0]);

    const size_t header_count = this->registor_field(splited_buffer);
    this->valid_headers();
    this->registor_entity_body(splited_buffer, header_count + 1);
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

    stream << '[' << get_formated_date() << "] '" << this->get_method() << "' '" << this->get_request_uri() << "' '" << this->get_protocol() << "'" << std::endl;
    stream << "    header : {" << std::endl;
    for (std::map<std::string, std::vector<std::string> >::iterator i = this->header_.begin(); i != this->header_.end(); i++) {
        stream << "        " << std::setw(20) << std::left << i->first << ": ";
        for (size_t j = 0; j < i->second.size(); j++) {
            stream << "'"<< i->second[j] << "', ";
        }
        stream << std::endl;
    }
    stream << "    }" << std::endl;
    stream << "    body : {" << std::endl;
    stream << "        " << this->entity_body_ << std::endl;
    stream << "    }" << std::endl;
    stream << std::left << std::setw(width) << "    Allow" << " : [";
    for (size_t i = 0; i < this->allow_.size(); i++) {
        stream << '"' << this->allow_.at(i) << '"' << ", ";
    }
    stream << "]" << std::endl;
    stream << std::left << std::setw(width) << "    Content-Encoding" << " : " << '"' << this->content_encoding_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Content-length" << " : " << '"' << this->content_length_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Content-Type" << " : " << '"' << this->content_type_.type << '/' << this->content_type_.subtype << '"' << " parameter : ";
    for (std::multimap<std::string, std::string>::iterator i = this->content_type_.parameter.begin(); i != this->content_type_.parameter.end(); i++) {
        stream << "{" << i->first << ":" << i->second << "}, ";
    }
    stream << std::endl;
    stream << std::left << std::setw(width) << "    Date" << " : " << '"' << this->date_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Expries" << " : " << '"' << this->expires_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Form" << " : " << '"' << this->form_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    If-Modified-Since" << " : " << '"' << this->if_modified_since_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Last-Modified" << " : " << '"' << this->last_modified_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    Pragma" << " : [";
    for (size_t i = 0; i < this->pragma_.size(); i++) {
        stream << "'" << this->pragma_.at(i) << "', ";
    }
    stream << "]" << std::endl;
    stream << std::left << std::setw(width) << "    Referer" << " : " << '"' << this->referer_ << '"' << std::endl;
    stream << std::left << std::setw(width) << "    User-Agent" << " : " << "[";
    for (size_t i = 0; i < this->user_agent_.size(); i++) {
        stream << "'" << this->user_agent_.at(i) << "', ";
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
    case kConvertFail:
        return "HTTPHeader: convert failed";
        break;
    default:
        return "HTTPHeader: unkonw error";
        break;
    }
}

