#pragma once

#include <map>
#include <vector>
#include <string>

class HTTPResponse {
 private:
    int               status_code_;
    std::string       description_;
    std::string       content_type_;
    std::string       body_;
    static const char kHTTPVersion[];

    // 同じ名前のヘッダーを生成したい時にこのほうが都合がいいのでmutlimap //
    std::multimap<std::string, std::string> extend_header_;

 public:
    enum StatusCode {
        kOK                   = 200,
        kBadRequest           = 400,
        kForbidden            = 403,
        kNotFound             = 404,
        kMethodNotAllowed     = 405,
        kPayloadTooLarge      = 413,
        kInternalServerErrror = 500,
        kNotImplemented       = 501
    };
    HTTPResponse();
    HTTPResponse(
        StatusCode  status_code,
        std::string content_type,
        std::string body
    );
    HTTPResponse(
        int         status_code,
        std::string content_type,
        std::string body
    );
    HTTPResponse(const HTTPResponse&);
    HTTPResponse& operator=(const HTTPResponse&);
    ~HTTPResponse();
    void AddHeader(const std::string &name, const std::string &value);
    std::string toString() const;
};

HTTPResponse::StatusCode convert_status_code_to_enum(const int &code);
