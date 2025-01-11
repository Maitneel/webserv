#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

class LocastionConfig {
 public :
    std::string             name_;  // url、 pathみたいな //
    std::string             document_root_;
    std::set<std::string>   methods_;
    bool                    autoindex_;
    std::string             cgi_path_;
    size_t                  max_body_size_;
    std::string             redirect_;

    const LocastionConfig operator=(const LocastionConfig &rhs);
};

struct ServerConfigKey {
 public:
    const int port_;
    const std::string server_name_;

    ServerConfigKey(const int &port, const std::string &name) : port_(port), server_name_(name) {}
};

bool operator<(const ServerConfigKey &lhs, const ServerConfigKey &rhs);

class ServerConfig {
 public :
    std::string server_name_;
    LocastionConfig common_config_;
    int         port_;
    std::map<std::string, LocastionConfig> location_configs_;  // <url, config> //
    std::map<int, std::string> error_page_path_;
};

std::map<ServerConfigKey, ServerConfig> parse_config(std::string path);  // <virtual-name, config> //
