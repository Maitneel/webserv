#include <sstream>
#include "http_response.hpp"

HTTPResponse::HTTPResponse(
    int status_code,
    std::map<std::string, std::string> headers,
    std::string body
):
status_code(status_code),
headers(headers),
body(body) {}

HTTPResponse::~HTTPResponse() {}

std::string HTTPResponse::toString() const {
    std::stringstream ss;

    // TODO(taksaito): Content-Type の判定や、description の文字の処理を実装。
    ss << "HTTP/1.1 " << this->status_code << " OK" << "\r\n";
    ss << "Content-Type: text/html\r\n";
    ss << "Content-Length: ";
    ss << body.length() << "\r\n";
    ss << "\r\n";
    ss << body;
    ss << "\r\n";
    return ss.str();
}
