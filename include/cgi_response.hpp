#ifndef INCLUDE_CGI_RESPONSE_HPP_
#define INCLUDE_CGI_RESPONSE_HPP_

#include <string>
#include <map>

#include "http_response.hpp"

class CGIResponse {
 public:
    enum ResponseType {
        kDocumentResponse,
        kLocalRedirectResponse,
        kClientRedirectResponse,
        kClientRedirectResponseWithDocument,
        kUnknownType
    };

 private:
    int status_code_;
    ResponseType type_;

    std::multimap<std::string, std::string> filed_;
    std::string body_;

    void add_header(const std::string &header_line);
    int register_status_code(const std::string &status_line);
    void register_body(std::vector<std::string> &splited_by_lf, const size_t &body_front);
 public:

    CGIResponse(const std::string &str);
    ~CGIResponse();
    HTTPResponse make_http_response();
};

#endif // INCLUDE_CGI_RESPONSE_HPP_