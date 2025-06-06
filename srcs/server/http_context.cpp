#include <cstring>
#include <string>
#include "http_context.hpp"
#include "http_validation.hpp"

HTTPContext::HTTPContext(int fd) : connection_fd_(fd), parsed_header_(false), parsed_body_(false), is_cgi_(false), file_fd_(-1), sent_response_(false), error_occured_(false), content_type() {
}


HTTPContext::~HTTPContext() {
}

int HTTPContext::GetConnectionFD() const {
    return connection_fd_;
}

const std::string& HTTPContext::GetBuffer() const {
    return buffer_;
}

bool HTTPContext::IsParsedHeader() const {
    return (this->parsed_header_);
}

bool HTTPContext::IsParsedBody() const {
    return (this->parsed_body_);
}

void HTTPContext::ParseRequestHeader(const int &port) {
    std::string header_str = buffer_.substr(0, buffer_.find("\r\n\r\n") + strlen(CRLF));

    request_.parse_request_header(header_str, port);
    body_.SetHeader(request_);
    body_.AddBuffer(buffer_.substr(buffer_.find("\r\n\r\n") + strlen("\r\n\r\n")));
    buffer_ = "";
    parsed_header_ = true;
}

void HTTPContext::ParseRequestBody() {
    request_.entity_body_ = body_.GetBody();
    body_ = HTTPRequestBody();
    this->parsed_body_ = true;
    // 適切な長さになるよにする(keep-aliveが動かない) //
    // buffer_ = buffer_.substr(request_.content_length_);
}

void HTTPContext::SetMaxBodySize(const size_t &max_body_size) {
    body_.SetMaxBodySize(max_body_size);
}

const HTTPRequest& HTTPContext::GetHTTPRequest() {
    return request_;
}

void HTTPContext::AppendBuffer(std::string str) {
    if (!parsed_header_) {
        buffer_ += str;
    } else {
        body_.AddBuffer(str);
    }
}
