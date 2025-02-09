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
        // 200
        kOK                   = 200,
        kCreated              = 201,
        kAccepted             = 202,
        kNoContent            = 204,

        // 300
        kMovedPermanently     = 301,
        kFound                = 302,

        // 400
        kBadRequest           = 400,
        kForbidden            = 403,
        kNotFound             = 404,
        kMethodNotAllowed     = 405,
        kRequestTimeout       = 408,
        kPayloadTooLarge      = 413,
        kTooManyRequests      = 429,

        // 500
        kInternalServerErrror = 500,
        kNotImplemented       = 501,
        kBadGateway           = 502,
        kServiceUnavailable   = 503,
        kGatewayTimeout       = 504,
        kLoopDetected         = 508,
        kNetworkAuthenticationRequired = 511,
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

std::string GenerateDescription(HTTPResponse::StatusCode status_code);
HTTPResponse::StatusCode convert_status_code_to_enum(const int &code);

HTTPResponse create_redirect_response(const std::string &redirecto_to);
