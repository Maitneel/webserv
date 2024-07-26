#pragma once

#include <map>
#include <string>

class HTTPResponse {
 private:
    int         status_code;
    std::string content_type;
    std::string body;
 public:
    HTTPResponse(
        int         status_code,
        std::string content_type,
        std::string body
    );
    ~HTTPResponse();
    std::string toString() const;
};
