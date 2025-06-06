#pragma once

#include <unistd.h>
#include <string>

#include "http_request.hpp"
#include "cgi.hpp"

class HTTPContext {
 private:
    const int connection_fd_;
    std::string buffer_;
    bool parsed_header_;
    bool parsed_body_;

 public:
    HTTPRequest request_;
    CGIInfo cgi_info_;
    bool is_cgi_;
    int file_fd_;
    bool sent_response_;
    bool error_occured_;
    std::string content_type;
    HTTPRequestBody body_;

    explicit HTTPContext(int fd);
    ~HTTPContext();
    int GetConnectionFD() const;
    const std::string& GetBuffer() const;
    bool IsParsedHeader() const;
    bool IsParsedBody() const;
    void ParseRequestHeader(const int &port);
    void ParseRequestBody();
    void SetMaxBodySize(const size_t &max_body_size);
    const HTTPRequest &GetHTTPRequest();
    void AppendBuffer(std::string str);
};
