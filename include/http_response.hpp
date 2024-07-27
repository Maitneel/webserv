#pragma once

#include <map>
#include <string>

class HTTPResponse {
 private:
    int         status_code;
    std::string description;
    std::string content_type;
    std::string body;
 public:
    enum StatusCode {
        kOK                   = 200,
        kBadRequest           = 400,
        kForbidden            = 403,
        kNotFound             = 404,
        kMethodNotAllowed     = 405,
        kInternalServerErrror = 500,
    };

    HTTPResponse(
        StatusCode  status_code,
        std::string content_type,
        std::string body
    );
    ~HTTPResponse();
    std::string toString() const;
};
