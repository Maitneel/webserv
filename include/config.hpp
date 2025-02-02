#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

struct LocatoinConfig {
 public :
    std::string             name_;  // url、 pathみたいな //
    std::string             document_root_;
    std::set<std::string>   methods_;
    std::set<std::string>   index_;
    bool                    autoindex_;
    std::string             cgi_path_;
    size_t                  max_body_size_;
    std::string             redirect_;

    std::string ToString();
    const LocatoinConfig operator=(const LocatoinConfig &rhs);
};

struct ServerConfigKey {
 public:
    const int port_;
    const std::string server_name_;

    ServerConfigKey(const int &port, const std::string &name) : port_(port), server_name_(name) {}
};

bool operator<(const ServerConfigKey &lhs, const ServerConfigKey &rhs);
bool operator==(const ServerConfigKey &lhs, const ServerConfigKey &rhs);

struct ServerConfig {
 public :
    std::string server_name_;
    int         port_;
    std::string error_page_;
    std::map<std::string, LocatoinConfig> location_configs_;  // <url, config> //
    std::map<int, std::string> error_page_path_;
    std::string ToString();
};

std::map<ServerConfigKey, ServerConfig> parse_config(std::string path);  // <virtual-name, config> //
