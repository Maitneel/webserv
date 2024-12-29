#include <string>
#include <vector>
#include <map>
#include <utility>

#include "config.hpp"

std::map<std::string, ServerConfig> parse_config(std::string path) {
    // TODO(taksaito): implement
    // 一旦固定値を返す
    std::map<std::string, ServerConfig> configs;
    {
        LocastionConfig location_conf;
        location_conf.document_root_ = "./docs";
        location_conf.methods_.push_back(GET);

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8080;
        server_config.server_name_ = "localshot";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(server_config.server_name_, server_config));
    }
    {
        LocastionConfig location_conf;
        location_conf.document_root_ = "./docs";
        location_conf.methods_.push_back(GET);

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8001;
        server_config.server_name_ = "localshot";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(server_config.server_name_, server_config));
    }

    (void)(path);
    return configs;
}
