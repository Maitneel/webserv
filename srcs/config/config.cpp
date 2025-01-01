#include <string>
#include <vector>
#include <map>
#include <utility>

#include "config.hpp"

bool operator<(const ServerConfigKey &lhs, const ServerConfigKey &rhs) {
    if (lhs.port_ != rhs.port_) {
        return (lhs.port_ < rhs.port_);
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
        location_conf.methods_.insert("GET");

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8080;
        server_config.server_name_ = "localhost";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }
    {
        LocastionConfig location_conf;
        location_conf.document_root_ = "./docs";
        location_conf.methods_.insert("GET");

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8001;
        server_config.server_name_ = "localhost";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }

    (void)(path);
    return configs;
}

std::pair<std::string, LocastionConfig> gen_loc_conf(
    std::string name,
    std::string doc_root,
    bool auto_index,
    std::string cgi_path
) {
    LocastionConfig conf;

    conf.name_ = name;

    conf.methods_.insert("GET");
    conf.methods_.insert("POST");
    conf.methods_.insert("DELETE");

    conf.document_root_ = doc_root;
    conf.autoindex_ = auto_index;
    conf.cgi_path_ = cgi_path;

    // conf.max_body_size_ // Not Implement for NUW
    // conf.redirect_  // Not Implement for NUW

    return std::make_pair(name, conf);
}

std::map<std::string, LocastionConfig> hard_coding_loc_config() {
    std::map<std::string, LocastionConfig> conf_map;
    conf_map.insert(gen_loc_conf("/", "./docs", true, ""));
    conf_map.insert(gen_loc_conf("/cgi/date.cgi", "", false, "./cgi_script/date/date.cgi"));
    conf_map.insert(gen_loc_conf("/cgi/echo.cgi", "", false, "./cgi_script/echo/echo.cgi"));
    conf_map.insert(gen_loc_conf("/cgi/message_board", "", false, "./cgi_script/message_board/message_board.cgi"));

    return conf_map;
}

std::map<ServerConfigKey, ServerConfig> hard_coding_config() {
    std::map<ServerConfigKey, ServerConfig> configs;
    {
        ServerConfig server_config;
        server_config.location_configs_ = hard_coding_loc_config();
        server_config.port_ = 8080;
        server_config.server_name_ = "localhost";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }
    {
        ServerConfig server_config;
        server_config.location_configs_ = hard_coding_loc_config();
        server_config.port_ = 8001;
        server_config.server_name_ = "localhost";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }
    {
        ServerConfig server_config;
        server_config.location_configs_ = hard_coding_loc_config();
        server_config.port_ = 8080;
        server_config.server_name_ = "127.0.0.1";
        server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }

    return configs;
}



