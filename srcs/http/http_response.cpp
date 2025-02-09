#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include "http_response.hpp"

const char HTTPResponse::kHTTPVersion[] = "HTTP/1.1";

std::string GenerateDescription(HTTPResponse::StatusCode status_code) {
    switch (status_code) {
        // 200 系
        case HTTPResponse::kOK:
            return "OK";
        case HTTPResponse::kCreated:
            return "Created";
        case HTTPResponse::kAccepted:
            return "Accepted";
        case HTTPResponse::kNoContent:
            return "No Content";
        // 300 系
        case HTTPResponse::kMovedPermanently:
            return "Moved Permanently";
        case HTTPResponse::kFound:
            return "Found";
        // 400 系
        case HTTPResponse::kBadRequest:
            return "Bad Request";
        case HTTPResponse::kForbidden:
            return "Forbidden";
        case HTTPResponse::kNotFound:
            return "Not Found";
        case HTTPResponse::kMethodNotAllowed:
            return "Method Not Allowed";
        case HTTPResponse::kRequestTimeout:
            return "Request Timeout";
        case HTTPResponse::kPayloadTooLarge:
            return "Payload Too Large";
        case HTTPResponse::kTooManyRequests:
            return "Too Many Requests";
        // 500 系
        case HTTPResponse::kInternalServerErrror:
            return "Internal Server Error";
        case HTTPResponse::kNotImplemented:
            return "Not Implemented";
        case HTTPResponse::kBadGateway:
            return "Bad Gateway";
        case HTTPResponse::kServiceUnavailable:
            return "Service Unavailable";
        case HTTPResponse::kGatewayTimeout:
            return "Gateway Timeout";
        case HTTPResponse::kLoopDetected:
            return "Loop Detected";
        case HTTPResponse::kNetworkAuthenticationRequired:
            return "Network Authentication Required";
    }
    throw std::runtime_error("unreachable code");
}

HTTPResponse::StatusCode convert_status_code_to_enum(const int &code) {
    if (code == 200) {
        return HTTPResponse::kOK;
    }
    if (code == 201) {
        return HTTPResponse::kCreated;
    }
    if (code == 202) {
        return HTTPResponse::kAccepted;
    }
    if (code == 204) {
        return HTTPResponse::kNoContent;
    }
    if (code == 301) {
        return HTTPResponse::kMovedPermanently;
    }
    if (code == 302) {
        return HTTPResponse::kFound;
    }
    if (code == 400) {
        return HTTPResponse::kBadRequest;
    }
    if (code == 403) {
        return HTTPResponse::kForbidden;
    }
    if (code == 404) {
        return HTTPResponse::kNotFound;
    }
    if (code == 405) {
        return HTTPResponse::kMethodNotAllowed;
    }
    if (code == 408) {
        return HTTPResponse::kRequestTimeout;
    }
    if (code == 413) {
        return HTTPResponse::kPayloadTooLarge;
    }
    if (code == 429) {
        return HTTPResponse::kTooManyRequests;
    }

    if (code == 500) {
        return HTTPResponse::kInternalServerErrror;
    }
    if (code == 501) {
        return HTTPResponse::kNotImplemented;
    }
    if (code == 502) {
        return HTTPResponse::kBadGateway;
    }
    if (code == 503) {
        return HTTPResponse::kServiceUnavailable;
    }
    if (code == 504) {
        return HTTPResponse::kGatewayTimeout;
    }
    if (code == 508) {
        return HTTPResponse::kLoopDetected;
    }
    if (code == 511) {
        return HTTPResponse::kNetworkAuthenticationRequired;
    }

    return HTTPResponse::kInternalServerErrror;
}

HTTPResponse::HTTPResponse(): status_code_(kOK), content_type_("text/html"), body_("") {}

HTTPResponse::HTTPResponse(
    HTTPResponse::StatusCode status_code,
    std::string    content_type,
    std::string    body
):
status_code_(status_code),
content_type_(content_type),
body_(body) {
    this->description_ = GenerateDescription(status_code);
    AddHeader("Connection", "Close");
}

HTTPResponse::HTTPResponse(
    int            status_code,
    std::string    content_type,
    std::string    body
):
status_code_(convert_status_code_to_enum(status_code)),
content_type_(content_type),
body_(body) {
    this->description_ = GenerateDescription(convert_status_code_to_enum(status_code));
    AddHeader("Connection", "Close");
}

HTTPResponse::HTTPResponse(const HTTPResponse& other) {
    *this = other;
}

HTTPResponse& HTTPResponse::operator=(const HTTPResponse& other) {
    if (this == &other)
        return *this;
    this->status_code_      = other.status_code_;
    this->description_      = other.description_;
    this->content_type_     = other.content_type_;
    this->body_             = other.body_;
    this->extend_header_    = other.extend_header_;
    return *this;
}

HTTPResponse::~HTTPResponse() {}

void HTTPResponse::AddHeader(const std::string &name, const std::string &value) {
    this->extend_header_.insert(std::make_pair(name, value));
}

std::string HTTPResponse::toString() const {
    std::stringstream ss;

    ss <<  HTTPResponse::kHTTPVersion << " " << this->status_code_ << " " << this->description_ << "\r\n";
    ss << "Content-Type: " << this->content_type_ << "\r\n";
    std::map<std::string, std::vector<std::string> >hoge;
    for (std::map<std::string, std::string>::const_iterator it = this->extend_header_.begin(); it != this->extend_header_.end(); it++) {
        ss << it->first << ": " << it->second << "\r\n";
    }
    if (extend_header_.find("Content-Length") == extend_header_.end() && this->body_.length() != 0) {
        ss << "Content-Length: ";
        ss << this->body_.length() << "\r\n";
    }

    // std::cerr << "res-----------------" << std::endl;
    // std::cerr << ss.str();

    ss << "\r\n";
    ss << this->body_;
    return ss.str();
}

HTTPResponse create_redirect_response(const std::string &redirecto_to) {
    HTTPResponse res(301, "text/plain", "Moved Permanently");
    std::string location_header = "Location: ";
    res.AddHeader("Location", redirecto_to);
    return res;
}
