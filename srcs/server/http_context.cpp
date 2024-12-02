#include <cstring>
#include <string>
#include "http_context.hpp"
#include "http_validation.hpp"

HTTPContext::HTTPContext(int fd) : connection_fd_(fd), parsed_header_(false) {
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

void HTTPContext::ParseRequestHeader() {
    std::string header_str = buffer_.substr(0, buffer_.find("\r\n\r\n") + strlen(CRLF));

    request_.parse_request_header(header_str);
    buffer_ = buffer_.substr(buffer_.find("\r\n\r\n") + strlen("\r\n\r\n"));
    parsed_header_ = true;
}

void HTTPContext::ParseRequestBody() {
    // TODO(maitneel): 直接触れないようにしたほうがいいかもしれない //
    request_.entity_body_ = buffer_.substr(0, request_.content_length_);
    buffer_ = buffer_.substr(request_.content_length_);
}

HTTPRequest& HTTPContext::GetHTTPRequest() {
    return request_;
}

void HTTPContext::AppendBuffer(std::string str) {
    buffer_ += str;
}
