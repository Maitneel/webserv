#pragma once

#include <unistd.h>
#include <string>

#include "http_request.hpp"
#include "cgi.hpp"

class HTTPContext {
 private:
    const int connection_fd_;
    std::string buffer_;
    HTTPRequest request_;
    bool parsed_header_;
    bool parsed_body_;

 public:
    CGIInfo cgi_info_;
    bool is_cgi_;
    int file_fd_;
    bool sent_response_;
    bool error_occured_;
    HTTPRequestBody body_;

    explicit HTTPContext(int fd);
    ~HTTPContext();
    int GetConnectionFD() const;
    const std::string& GetBuffer() const;
    bool IsParsedHeader() const;
    bool IsParsedBody() const;
    void ParseRequestHeader(const int &port);
    void ParseRequestBody();
    const HTTPRequest &GetHTTPRequest();
    void AppendBuffer(std::string str);
};
