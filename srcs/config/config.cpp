#include <string>
#include <vector>
#include <map>
#include <utility>

#include "config.hpp"

bool operator<(const ServerConfigKey &lhs, const ServerConfigKey &rhs) {
    if (lhs.port_ < rhs.port_) {
        return true;
    }
    return (lhs.server_name_ < rhs.server_name_);
}

std::map<ServerConfigKey, ServerConfig> parse_config(std::string path) {
    // TODO(taksaito): implement
    // 一旦固定値を返す
    std::map<ServerConfigKey, ServerConfig> configs;
    {
        LocastionConfig location_conf;
        location_conf.document_root_ = "./docs";
        location_conf.methods_.push_back("GET");

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8080;
        server_config.server_name_ = "localshot";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }
    {
        LocastionConfig location_conf;
        location_conf.document_root_ = "./docs";
        location_conf.methods_.push_back("GET");

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8001;
        server_config.server_name_ = "localshot";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }

    (void)(path);
    return configs;
}
