#pragma once

#include <string>
#include <vector>

enum HTTPMethod {
    GET = 0,
    POST,
    DELETE,
};

class LocastionConfig {
 public :
    std::string             name;
    std::string             document_root;
    std::vector<HTTPMethod> methods;
    bool                    autoindex;
    std::string             cgi_path;
    size_t                  max_body_size;
    std::string             redirect;
};

class ServerConfig {
 public :
    std::vector<std::string> server_names;
    std::string document_root;
    int         port;
    std::vector<LocastionConfig> location_configs;
};

std::vector<ServerConfig> parse_config(std::string path);
