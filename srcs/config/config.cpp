#include <string>
#include <vector>
#include <map>
#include <utility>
#include <sstream>

#include "config.hpp"

LocationConfig::LocationConfig():
name_(""),
document_root_(""),
autoindex_(false),
cgi_path_(""),
max_body_size_(100),
redirect_("") {}

const LocationConfig LocationConfig::operator=(const LocationConfig &rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->name_ =  rhs.name_;
    this->document_root_ =  rhs.document_root_;
    this->methods_ =  rhs.methods_;
    this->autoindex_ =  rhs.autoindex_;
    this->cgi_path_ =  rhs.cgi_path_;
    this->max_body_size_ =  rhs.max_body_size_;
    this->redirect_ =  rhs.redirect_;

    return *this;
}

bool operator<(const ServerConfigKey &lhs, const ServerConfigKey &rhs) {
    if (lhs.port_ != rhs.port_) {
        return (lhs.port_ < rhs.port_);
    }
    return (lhs.server_name_ < rhs.server_name_);
}

bool operator==(const ServerConfigKey &lhs, const ServerConfigKey &rhs) {
    return lhs.port_ == rhs.port_ && lhs.server_name_ == rhs.server_name_;
}

std::string LocationConfig::ToString() {
    std::stringstream ss;
    ss << "\t" << "name: " << name_ << std::endl;
    ss << "\t" << "method: ";
    for (std::set<std::string>::iterator it = methods_.begin(); it != methods_.end(); it++) {
        ss << *it << " ";
    }
    ss << std::endl;
    ss << "\t" << "document_root: " << document_root_ << std::endl;
    // ss << "\t" << "methods: " << methods_ << std::endl;
    ss << "\t" << "index: ";
    for (std::set<std::string>::iterator it = index_.begin(); it != index_.end(); it++) {
        ss << *it << " ";
    }
    ss << std::endl;
    ss << "\t" << "autoindex:  " << std::boolalpha << autoindex_ << std::endl;
    ss << "\t" << "cgi_path:  " << cgi_path_ << std::endl;
    ss << "\t" << "max_body_size:" << max_body_size_ << std::endl;
    ss << "\t" << "redirect:" << redirect_ << std::endl;
    return ss.str();
}

std::string ServerConfig::ToString() {
    std::stringstream ss;
    ss << "server_name: " << server_name_ << std::endl;
    ss << "port: " << port_ << std::endl;
    for (std::map<int, std::string>::iterator it = error_page_path_.begin(); it != error_page_path_.end(); it++) {
        ss << "error_page: " << it->first << " " << it->second << std::endl;
    }

    for (std::map<std::string, LocationConfig>::iterator it=location_configs_.begin(); it != location_configs_.end(); it++) {
        ss << it->first << std::endl;
        ss << it->second.ToString() << std::endl;
    }
    return ss.str();
}
