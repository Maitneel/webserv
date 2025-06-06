#include <string>

#include "http_exception.hpp"
#include "extend_stdlib.hpp"

MustReturnHTTPStatus::MustReturnHTTPStatus(const int &status_code) : status_code_(status_code) {
}

const char *MustReturnHTTPStatus::what() const throw() {
    const char *message = "MustReturnHTTPStatus: http-responce MUST responce with status-code";
    // std::string message = "MustReturnHTTPStatus: http-responce MUST responce with status-code ";
    // message += int_to_string(status_code_);

    return message;
}

const int &MustReturnHTTPStatus::GetStatusCode() const throw() {
    return status_code_;
}
