#pragma once

#include <map>
#include <string>

class HTTPResponse {
 private:
    int                                status_code;
    std::map<std::string, std::string> headers;
    std::string                        body;
 public:
    HTTPResponse(
        int status_code,
        std::map<std::string, std::string> headers,
        std::string body
    );
    ~HTTPResponse();
    std::string toString() const;
};
