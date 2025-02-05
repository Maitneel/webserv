#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include "http_response.hpp"

const char HTTPResponse::kHTTPVersion[] = "HTTP/1.1";

std::string GenerateDescription(HTTPResponse::StatusCode status_code) {
    switch (status_code) {
        case HTTPResponse::kOK:
            return "OK";
        case HTTPResponse::kCreated:
            return "Created";
        case HTTPResponse::kBadRequest:
            return "Bad Request";
        case HTTPResponse::kForbidden:
            return "Forbidden";
        case HTTPResponse::kNotFound:
            return "Not Found";
        case HTTPResponse::kMethodNotAllowed:
            return "Method Not Allowed";
        case HTTPResponse::kPayloadTooLarge:
            return "Payload Too Large";
        case HTTPResponse::kRequestTimeout:
            return "Request Timeout";
        case HTTPResponse::kInternalServerErrror:
            return "Internal Server Error";
        case HTTPResponse::kNotImplemented:
            return "Not Implemented";
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
    if (code == 500) {
        return HTTPResponse::kInternalServerErrror;
    }
    if (code == 501) {
        return HTTPResponse::kNotImplemented;
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

#include <iostream>
using namespace std;

std::string HTTPResponse::toString() const {
    std::stringstream ss;

    // TODO(taksaito): Content-Type の判定や、description の文字の処理を実装。
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

    // cerr << "res-----------------" << endl;
    // cerr << ss.str();

    ss << "\r\n";
    ss << this->body_;
    return ss.str();
}
