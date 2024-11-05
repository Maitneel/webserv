#pragma once

#include <unistd.h>
#include <string>

#include "http_request.hpp"

class HTTPContext {
 private:
    const int socket_fd_;
    std::string buffer_;
    pid_t cgi_pid_;
    HTTPRequest request_;
    bool parsed_header_;
 public:
    explicit HTTPContext(int fd);
    ~HTTPContext();
    int GetSocketFD() const;
    const std::string& GetBuffer() const;
    bool IsParsedHeader() const;
    void ParseRequestHeader();
    void ParseRequestBody();
    // TODO(maitneel): const で返すようにする //
    HTTPRequest& GetHTTPRequest();
    void AppendBuffer(std::string str);
};
