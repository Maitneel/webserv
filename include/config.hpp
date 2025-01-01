#pragma once

#include <string>
#include <vector>
#include <map>

class LocastionConfig {
 public :
    std::string             name_;  // url、 pathみたいな //
    std::string             document_root_;
    std::vector<std::string> methods_;
    bool                    autoindex_;
    std::string             cgi_path_;
    size_t                  max_body_size_;
    std::string             redirect_;
};

struct ServerConfigKey {
 public:
    const int port_;
    const std::string server_name_;

    ServerConfigKey(const int &port, const std::string &name) : port_(port), server_name_(name) {};

};

class ServerConfig {
 public :
    std::string server_name_;
    std::string document_root_;
    int         port_;
    std::map<std::string, LocastionConfig> location_configs_;  // <url, config> //
};

std::map<ServerConfigKey, ServerConfig> parse_config(std::string path);  // <virtual-name, config> //
