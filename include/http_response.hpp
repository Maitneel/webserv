#pragma once

#include <map>
#include <string>

class HTTPResponse {
 private:
    int               status_code_;
    std::string       description_;
    std::string       content_type_;
    std::string       body_;
    static const char kHTTPVersion[];

 public:
    enum StatusCode {
        kOK                   = 200,
        kBadRequest           = 400,
        kForbidden            = 403,
        kNotFound             = 404,
        kMethodNotAllowed     = 405,
        kInternalServerErrror = 500,
        kNotImplemented       = 501
    };
    HTTPResponse();
    HTTPResponse(
        StatusCode  status_code,
        std::string content_type,
        std::string body
    );
    HTTPResponse(const HTTPResponse&);
    HTTPResponse& operator=(const HTTPResponse&);
    ~HTTPResponse();
    std::string toString() const;
};
