#include <cstring>
#include <string>
#include <map>
#include <utility>
#include <vector>

#include "cgi_response.hpp"
#include "http_response.hpp"
#include "extend_stdlib.hpp"
#include "http_validation.hpp"

static std::string get_location(const std::string &first_line) {
    std::string::size_type front = first_line.find_first_not_of(" :", strlen("Location"));
    std::string location;
    try {
        location = first_line.substr(front);
    } catch (...) {
    }
    return location;
}

// document-response        = Content-Type [ Status ] *other-field NL
//                            response-body
// local-redir-response     = local-Location NL
// client-redir-response    = client-Location *extension-field NL
// client-redirdoc-response = client-Location Status Content-Type
//                            *other-field NL response-body
static CGIResponse::ResponseType get_response_type(const std::vector<std::string> &splited_buffer) {
    if (splited_buffer.at(0).find("Content-Type") == 0) {
        return CGIResponse::kDocumentResponse;
    } else if (splited_buffer.at(0).find("Location") != 0) {
        // 実質exceptionなのでなんかthrowしてもいい //
        return CGIResponse::kUnknownType;
    }
    std::string location = get_location(splited_buffer.at(0));
    if (location.at(0) == '/') {
        return CGIResponse::kLocalRedirectResponse;
    } else {
        // withDocument かどうかは調べればわかるが、この時点でしらべるのは非効率なのでヘッダーの処理後にbodyがあるかで決定する //
        // include CGIResponse::kClientRedirectResponseWithDocument;
        return CGIResponse::kClientRedirectResponse;
    }
}

void CGIResponse::add_header(const std::string &header_line) {
    std::string key = header_line.substr(0, header_line.find(":"));
    std::string value;
    try {
        value = header_line.substr(header_line.find_first_not_of(": ", key.length()));
        if (*(value.end() - 1) == '\n') {
            value.erase(value.end() - 1);
        }
        if (*(value.end() - 1) == '\r') {
            value.erase(value.end() - 1);
        }
    } catch (...) {
    }
    this->filed_.insert(std::make_pair(key, value));
}

bool is_status_code_line(const std::string &line) {
    const std::string status_str = "Status:";
    try {
        if (line.substr(0, status_str.length()) != status_str) {
            return false;
        }
        std::string status_code_str = line.substr(line.find_first_not_of(" ", line.find_first_not_of(status_str)));
        if ('0' <= status_code_str.at(0) && status_code_str.at(0) <= '5' && is_digit(status_code_str.at(1)) && is_digit(status_code_str.at(2)) && status_code_str.at(3) == ' ') {
            return true;
        }
    } catch (...) {
    }
    return false;
}

int CGIResponse::register_status_code(const std::string &status_line) {
    try {
        if (is_status_code_line(status_line)) {
            std::string status_code_str = status_line.substr(status_line.find_first_not_of(" ", status_line.find_first_not_of("Status:")));
            status_code_ = 0;
            this->status_code_ += (status_code_str.at(0) - '0') * 100;
            this->status_code_ += (status_code_str.at(1) - '0') * 10;
            this->status_code_ += (status_code_str.at(2) - '0') * 1;
            return 1;
        } else {
            status_code_ = 200;
        }
    } catch (...) {
    }
    return 0;
}

void CGIResponse::register_body(const std::vector<std::string> &splited_by_lf, const size_t &body_front) {
    for (size_t i = body_front; i < splited_by_lf.size(); i++) {
        this->body_ += splited_by_lf[i];
    }
}

CGIResponse::CGIResponse(const std::string &buffer) {
    std::vector<std::string> splited_by_lf = split(buffer, "\n");
    if (splited_by_lf[0].find("Status:") != std::string::npos) {
        splited_by_lf.erase(splited_by_lf.begin());
    }
    this->type_ = get_response_type(splited_by_lf);
    add_header(splited_by_lf.at(0));
    if (this->type_ == kLocalRedirectResponse) {
        // if (2 < splited_by_lf.size()) の場合エラーなのでthrowしてもいいがめんどくさいのでしない
        return;
    }
    size_t line_index = 1;
    if (this->type_ == kDocumentResponse) {
        line_index += this->register_status_code(splited_by_lf[line_index]);
    }
    // ここキモいかも　//
    for (; line_index < splited_by_lf.size(); line_index++) {
        if (splited_by_lf.at(line_index) == "\n" || splited_by_lf.at(line_index) == "\r\n") {
            break;
        }
        add_header(splited_by_lf.at(line_index));
    }
    line_index++;
    this->register_body(splited_by_lf, line_index);
    if (this->type_ == kClientRedirectResponse && this->body_ != "") {
        this->type_ = kClientRedirectResponseWithDocument;
    }
}

CGIResponse::~CGIResponse() {
}

HTTPResponse CGIResponse::make_http_response() {
    std::string content_type;
    if (this->filed_.find("Content-Type") != this->filed_.end()) {
        content_type = this->filed_.find("Content-Type")->second;
    } else {
        // ない場合どうすrのがいいのかよくわからん
        content_type = "text/plain";
    }
    HTTPResponse http_res(this->status_code_, content_type, this->body_);
    for (std::multimap<std::string, std::string>::iterator it = this->filed_.begin(); it != this->filed_.end(); it++) {
        if (it->first != "Content-Type") {
            http_res.AddHeader(it->first, it->second);
        }
    }
    http_res.AddHeader("Connection", "Close");
    return http_res;
}
