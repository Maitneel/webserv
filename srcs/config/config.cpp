#include "config.hpp"

std::vector<ServerConfig> parse_config(std::string path) {
    // TODO: implement
    // 一旦固定値を返す

    LocastionConfig location_conf;
    location_conf.document_root = "docs";
    location_conf.methods.push_back(GET);

    ServerConfig server_config;
    server_config.location_configs.push_back(location_conf);
    server_config.port = 8080;
    server_config.server_names.push_back("localshot");

    std::vector<ServerConfig> configs;
    configs.push_back(server_config);
    
    return configs;
}
