#include <cstring>
#include "http_context.hpp"

HTTPContext::HTTPContext(int fd) : socket_fd_(fd), parsed_header_(false) {
}


HTTPContext::~HTTPContext(){

}

int HTTPContext::GetSocketFD() const { 
    return socket_fd_;
}

const std::string& HTTPContext::GetBuffer() const {
    return buffer_;
}

bool HTTPContext::IsParsedHeader() const {
    return (this->parsed_header_);
}

void HTTPContext::ParseRequestHeader() {
    std::string header_str = buffer_.substr(0, buffer_.find("\r\n\r\n"));

    // TODO(maitneel): 新規でインスタンスを作らないようにする //
    request_ = HTTPRequest(header_str);
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
