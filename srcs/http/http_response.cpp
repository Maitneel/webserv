#include <sstream>
#include "http_response.hpp"

std::string GenerateDescription(HTTPResponse::StatusCode status_code) {
    switch (status_code) {
        case HTTPResponse::kOK:
            return "OK";
        case HTTPResponse::kBadRequest:
            return "Bad Request";
        case HTTPResponse::kForbidden:
            return "Forbidden";
        case HTTPResponse::kNotFound:
            return "Not Found";
        case HTTPResponse::kMethodNotAllowed:
            return "Method Not Allowed";
        case HTTPResponse::kInternalServerErrror:
            return "Internal Server Error";
    }
    throw std::runtime_error("unreachable code");
}

HTTPResponse::HTTPResponse(
    HTTPResponse::StatusCode status_code,
    std::string    content_type,
    std::string    body
):
status_code(status_code),
content_type(content_type),
body(body) {
    this->description = GenerateDescription(status_code);
}

HTTPResponse::~HTTPResponse() {}

std::string HTTPResponse::toString() const {
    std::stringstream ss;

    // TODO(taksaito): Content-Type の判定や、description の文字の処理を実装。
    ss << "HTTP/1.1 " << this->status_code << " " << this->description << "\r\n";
    ss << "Content-Type: " << this->content_type << "\r\n";
    ss << "Content-Length: ";
    ss << body.length() << "\r\n";
    ss << "\r\n";
    ss << body;
    ss << "\r\n";
    return ss.str();
}
