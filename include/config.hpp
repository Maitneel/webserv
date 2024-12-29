#pragma once

#include <string>
#include <vector>
#include <map>

enum HTTPMethod {
    GET = 0,
    POST,
    DELETE
};

class LocastionConfig {
 public :
    std::string             name_;  // url、 pathみたいな //
    std::string             document_root_;
    std::vector<HTTPMethod> methods_;
    bool                    autoindex_;
    std::string             cgi_path_;
    size_t                  max_body_size_;
    std::string             redirect_;
};

class ServerConfig {
 public :
    std::string server_name_;
    std::string document_root_;
    int         port_;
    std::map<std::string, LocastionConfig> location_configs_;  // <url, config> //
};

std::map<std::string, ServerConfig> parse_config(std::string path);  // <virtual-name, config> //
