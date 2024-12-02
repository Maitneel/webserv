#pragma once

#include <unistd.h>
#include <string>

#include "http_request.hpp"
#include "cgi.hpp"

class HTTPContext {
 private:
    const int connection_fd_;
    std::string buffer_;
    CGIInfo cgi_info_;
    HTTPRequest request_;
    bool parsed_header_;
 public:
    explicit HTTPContext(int fd);
    ~HTTPContext();
    int GetConnectionFD() const;
    const std::string& GetBuffer() const;
    bool IsParsedHeader() const;
    void ParseRequestHeader();
    void ParseRequestBody();
    // TODO(maitneel): const で返すようにする //
    HTTPRequest& GetHTTPRequest();
    void AppendBuffer(std::string str);
};
