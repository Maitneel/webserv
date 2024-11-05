#include <string>
#include <vector>

#include "config.hpp"

std::vector<ServerConfig> parse_config(std::string path) {
    // TODO(taksaito): implement
    // 一旦固定値を返す
    std::vector<ServerConfig> configs;
    {
        LocastionConfig location_conf;
        location_conf.document_root = "./docs";
        location_conf.methods.push_back(GET);

        ServerConfig server_config;
        server_config.location_configs.push_back(location_conf);
        server_config.port = 8080;
        server_config.server_names.push_back("localshot");
        server_config.document_root = "./docs";

        configs.push_back(server_config);
    }
    {
        LocastionConfig location_conf;
        location_conf.document_root = "./docs";
        location_conf.methods.push_back(GET);

        ServerConfig server_config;
        server_config.location_configs.push_back(location_conf);
        server_config.port = 8001;
        server_config.server_names.push_back("localshot");
        server_config.document_root = "./docs";

        configs.push_back(server_config);
    }

    return configs;
}
