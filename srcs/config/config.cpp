#include <string>
#include <vector>
#include <map>
#include <utility>
#include <sstream>

#include "config.hpp"

const LocationConfig LocationConfig::operator=(const LocationConfig &rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->name_ =  rhs.name_;
    this->document_root_ =  rhs.document_root_;
    this->methods_ =  rhs.methods_;
    this->autoindex_ =  rhs.autoindex_;
    this->cgi_path_ =  rhs.cgi_path_;
    this->max_body_size_ =  rhs.max_body_size_;
    this->redirect_ =  rhs.redirect_;

    return *this;
}

bool operator<(const ServerConfigKey &lhs, const ServerConfigKey &rhs) {
    if (lhs.port_ != rhs.port_) {
        return (lhs.port_ < rhs.port_);
    }
    return (lhs.server_name_ < rhs.server_name_);
}

bool operator==(const ServerConfigKey &lhs, const ServerConfigKey &rhs) {
    return lhs.port_ == rhs.port_ && lhs.server_name_ == rhs.server_name_;
}

std::string LocationConfig::ToString() {
    std::stringstream ss;
    ss << "\t" << "name: " << name_ << std::endl;
    ss << "\t" << "method: ";
    for (std::set<std::string>::iterator it = methods_.begin(); it != methods_.end(); it++) {
        ss << *it << " ";
    }
    ss << std::endl;
    ss << "\t" << "document_root: " << document_root_ << std::endl;
    // ss << "\t" << "methods: " << methods_ << std::endl;
    ss << "\t" << "index: ";
    for (std::set<std::string>::iterator it = index_.begin(); it != index_.end(); it++) {
        ss << *it << " ";
    }
    ss << std::endl;
    ss << "\t" << "autoindex:  " << std::boolalpha << autoindex_ << std::endl;
    ss << "\t" << "cgi_path:  " << cgi_path_ << std::endl;
    ss << "\t" << "max_body_size:" << max_body_size_ << std::endl;
    ss << "\t" << "redirect:" << redirect_ << std::endl;
    return ss.str();
}

std::map<ServerConfigKey, ServerConfig> parse_config(std::string path) {
    // TODO(taksaito): implement
    // 一旦固定値を返す
    std::map<ServerConfigKey, ServerConfig> configs;
    {
        LocationConfig location_conf;
        location_conf.document_root_ = "./docs";
        location_conf.methods_.insert("GET");

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8080;
        server_config.server_name_ = "localhost";
        // server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }
    {
        LocationConfig location_conf;
        location_conf.document_root_ = "./docs";
        location_conf.methods_.insert("GET");

        ServerConfig server_config;
        server_config.location_configs_.insert(std::make_pair(location_conf.name_, location_conf));
        server_config.port_ = 8001;
        server_config.server_name_ = "localhost";
        // server_config.document_root_ = "./docs";

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }

    (void)(path);
    return configs;
}

std::pair<std::string, LocationConfig> gen_loc_conf(
    std::string name,
    std::string doc_root,
    bool auto_index,
    std::string cgi_path
) {
    LocationConfig conf;

    conf.name_ = name;

    conf.methods_.insert("GET");
    // conf.methods_.insert("POST");
    // conf.methods_.insert("HEAD");
    conf.methods_.insert("DELETE");

    conf.document_root_ = doc_root;
    conf.autoindex_ = auto_index;
    conf.cgi_path_ = cgi_path;

    // conf.max_body_size_ // Not Implement for NUW
    // conf.redirect_  // Not Implement for NUW

    return std::make_pair(name, conf);
}

std::map<std::string, LocationConfig> hard_coding_loc_config() {
    std::map<std::string, LocationConfig> conf_map;
    conf_map.insert(gen_loc_conf("/", "./docs", true, ""));
    conf_map.insert(gen_loc_conf("/cgi/date.cgi/", "", false, "./cgi_script/date/date.cgi"));
    conf_map.insert(gen_loc_conf("/cgi/echo.cgi/", "", false, "./cgi_script/echo/echo.cgi"));
    conf_map.insert(gen_loc_conf("/cgi/message_board/", "", false, "./cgi_script/message_board/message_board.cgi"));
    conf_map.insert(gen_loc_conf("/cgi/cgi_test/", "", false, "./test/cgi_tester"));
    conf_map.insert(gen_loc_conf("/cgi/php/", "", false, "./docs/cgi/php_cgi.php"));
    conf_map.insert(gen_loc_conf("/cgi/python/", "", false, "./docs/cgi/python_cgi.py"));
    conf_map.insert(gen_loc_conf("/cgi/return_ok/", "", false, "./docs/cgi/return_ok.py"));

    conf_map.insert(gen_loc_conf("/hoge/", "./docs/dir_1/", false, ""));
    conf_map.insert(gen_loc_conf("/hoge/dir_2/", "./docs/dir_2/", false, ""));
    conf_map.insert(gen_loc_conf("/hoge/dir_3/", "./docs/dir_3/", false, ""));
    conf_map.insert(gen_loc_conf("/hoge/dir_5/", "./docs/dir_4/dir_5/", false, ""));
    conf_map.insert(gen_loc_conf("/hoge/dir_6/", "./docs/dir_4/dir_6/", false, ""));

    return conf_map;
}

std::map<ServerConfigKey, ServerConfig> hard_coding_config() {
    std::map<ServerConfigKey, ServerConfig> configs;
    {
        ServerConfig server_config;
        server_config.location_configs_ = hard_coding_loc_config();
        server_config.port_ = 8080;
        server_config.server_name_ = "localhost";
        server_config.location_configs_.insert(gen_loc_conf("/", "./docs/", false, ""));

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }
    {
        ServerConfig server_config;
        server_config.location_configs_ = hard_coding_loc_config();
        server_config.port_ = 8001;
        server_config.server_name_ = "localhost";
        server_config.location_configs_.insert(gen_loc_conf("/", "./docs/", false, ""));

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }
    {
        ServerConfig server_config;
        server_config.location_configs_ = hard_coding_loc_config();
        server_config.port_ = 8080;
        server_config.server_name_ = "127.0.0.1";
        server_config.location_configs_.insert(gen_loc_conf("/", "./docs/", false, ""));

        configs.insert(std::make_pair(ServerConfigKey(server_config.port_, server_config.server_name_), server_config));
    }

    return configs;
}

std::string ServerConfig::ToString() {
    std::stringstream ss;
    ss << "server_name: " << server_name_ << std::endl;
    ss << "port: " << port_ << std::endl;
    for (std::map<int, std::string>::iterator it = error_page_path_.begin(); it != error_page_path_.end(); it++) {
        ss << "error_page: " << it->first << " " << it->second << std::endl;
    }

    for (std::map<std::string, LocationConfig>::iterator it=location_configs_.begin(); it != location_configs_.end(); it++) {
        ss << it->first << std::endl;
        ss << it->second.ToString() << std::endl;
    }
    return ss.str();
}
