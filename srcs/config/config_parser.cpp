#include "config_parser.hpp"
#include "extend_stdlib.hpp"
#include <string>
#include <fstream>
#include <map>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <utility>

bool is_positive_number(const std::string& str) {
    std::string::const_iterator it = str.begin();
    while (it != str.end()) {
        if (!std::isdigit(*it))
            return false;
        it++;
    }
    return true;
}

bool is_start_with(const std::string& str, const std::string& start) {
    size_t idx = 0;
    size_t min_len = std::min(str.size(), start.size());

    while (idx < min_len && str[idx] == start[idx]) {
        idx++;        
    }
    if (start.size() != idx)
        return false;
    return true;
}

void rstrip(std::string* str_p) {
    std::string& str = *str_p;
    std::string::size_type pos = str.find_last_not_of("\t\n\v\f\r ");
    if(pos != std::string::npos)
        str = str.substr(0, pos + 1);
}

ConfigParser::ConfigParser(const std::string &file_path): read_index_(0) {
    std::ifstream ifs(file_path.c_str());
    if (ifs == NULL) {
        throw std::runtime_error(std::string("can't open ") + file_path);
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    raw_str_ = ss.str();
    rstrip(&raw_str_);
}

bool ConfigParser::is_end() {
    return read_index_ >= raw_str_.size();
}

std::string ConfigParser::GetToken() {
    std::string token;
    size_t idx = read_index_;
    while (idx < raw_str_.length() && isspace(raw_str_[idx])) {
        idx++;
    }
    do {
        token += raw_str_[idx];
        idx++;
    } while(idx < raw_str_.length() && !isspace(raw_str_[idx]) && raw_str_[idx] != ';');
    return token;
}

std::string ConfigParser::ConsumeToken() {
    while (read_index_ < raw_str_.length() && isspace(raw_str_[read_index_])) {
        if (raw_str_[read_index_] == '\n')
            current_line_++;
        read_index_++;
    }
    std::string token = GetToken();
    read_index_ += token.size();

    return token;
}

void ConfigParser::Consume(const std::string& expect) {
    std::string token = ConsumeToken();
    if (token != expect) {
        throw InvalidConfigException(current_line_, "expect " + expect + " but obtain " + token);
    }
}

void ConfigParser::valid_location_path(const std::string& path) {
    if (!(is_start_with(path, "/")))
        throw InvalidConfigException(current_line_, "location path must start \"/\"");
    if (path[path.size()-1] != '/')
        throw InvalidConfigException(current_line_, "location path must end \"/\"");
}

void ConfigParser::valid_url(const std::string& url) {
    if (!(is_start_with(url, "http://") || is_start_with(url, "https://")))
        throw InvalidConfigException(current_line_, "url must start \"http://\" or \"https://\"");
}

void ConfigParser::valid_path(const std::string& path) {
    if (!(is_start_with(path, "./") || is_start_with(path, "/")))
        throw InvalidConfigException(current_line_, "path must start ./ or /");
    if (path[path.size()-1] != '/')
        throw InvalidConfigException(current_line_, "path must end /");
}

void ConfigParser::valid_cgi_path(const std::string& path) {
    if (!(is_start_with(path, "./") || is_start_with(path, "/")))
        throw InvalidConfigException(current_line_, "path must start ./ or /");
}

void ConfigParser::parse_url(LocatoinConfig *location_config) {
    const std::string url = ConsumeToken();
    valid_url(url);
    location_config->redirect_ = url;
}

void ConfigParser::parse_return_directive(LocatoinConfig *location_config) {
    Consume("return");
    parse_url(location_config);
    Consume(";");
}

void ConfigParser::parse_cgi_path(LocatoinConfig *location_config) {
    Consume("cgi_path");
    const std::string cgi_path = ConsumeToken();
    valid_cgi_path(cgi_path);
    location_config->cgi_path_ = cgi_path;
    Consume(";");
}

void ConfigParser::parse_max_body_size(LocatoinConfig *location_config) {
    std::string body_size_str = ConsumeToken();
    if (!is_positive_number(body_size_str))
        throw InvalidConfigException(current_line_, "max_body_size is positve number");
    try {
        location_config->max_body_size_ = safe_atoi(body_size_str);
    } catch (const std::overflow_error &e) {
        InvalidConfigException(current_line_, "max_body_size is overflow.");
    } catch (const std::runtime_error &e) {
        InvalidConfigException(current_line_, "max_body_size must positive number");
    }
}

void ConfigParser::parse_max_body_size_directive(LocatoinConfig *location_config) {
    Consume("max_body_size");
    parse_max_body_size(location_config);
    Consume(";");
}

void ConfigParser::parse_autoindex_directive(LocatoinConfig *location_config) {
    Consume("autoindex");
    std::string token = ConsumeToken();
    if (token == "on")
        location_config->autoindex_ = true;
    else if (token == "off")
        location_config->autoindex_ = false;
    else
        throw InvalidConfigException(current_line_, "autoindex expect 'on' or 'off' but obtained " + token);
    Consume(";");
}

void ConfigParser::parse_index_files(LocatoinConfig *location_config) {
    do {
        std::string index_file = ConsumeToken();
        location_config->index_.insert(index_file);
    } while(GetToken() != ";");
}

void ConfigParser::parse_index_directive(LocatoinConfig *location_config) {
    Consume("index");
    parse_index_files(location_config);
    Consume(";");
}

void ConfigParser::parse_path(LocatoinConfig *location_config) {
    std::string root = ConsumeToken();
    valid_path(root);
    location_config->document_root_ = root;
}

void ConfigParser::parse_root_directive(LocatoinConfig *location_config) {
    Consume("root");
    parse_path(location_config);
    Consume(";");
}

void ConfigParser::parse_method(LocatoinConfig *location_config) {
    do {
        location_config->methods_.insert(ConsumeToken());
    } while(GetToken() != ";");
}

void ConfigParser::parse_method_directive(LocatoinConfig *location_config) {
    Consume("method");
    parse_method(location_config);
    Consume(";");
}

void ConfigParser::parse_location_path(LocatoinConfig *location_config) {
    const std::string path = ConsumeToken();
    valid_location_path(path);
    location_config->name_ = path;
}

void ConfigParser::parse_location_directive(ServerConfig *server_config) {
    LocatoinConfig location_config;
    Consume("location");
    parse_location_path(&location_config);
    Consume("{");
    if (GetToken() == "method") {
        parse_method_directive(&location_config);
        if (GetToken() == "root")
            parse_root_directive(&location_config);
        if (GetToken() == "index")
            parse_index_directive(&location_config);
        if (GetToken() == "autoindex")
            parse_autoindex_directive(&location_config);
        if (GetToken() == "max_body_size")
            parse_max_body_size_directive(&location_config);
        if (GetToken() == "cgi_path")
            parse_cgi_path(&location_config);
    } else {
        parse_return_directive(&location_config);
    }
    Consume("}");
    if (server_config->location_configs_.find(location_config.name_) != server_config->location_configs_.end())
        throw InvalidConfigException(current_line_, "duplicate location route");
    server_config->location_configs_.insert(std::make_pair(location_config.name_, location_config));
}

void ConfigParser::parse_location_directives(ServerConfig *server_config) {
    do {
        parse_location_directive(server_config);
    } while(GetToken() == "location");
}

void ConfigParser::parse_port(ServerConfig *server_config) {
    std::string port = this->ConsumeToken();
    if (!is_positive_number(port))
        throw InvalidConfigException(current_line_, "port must be positve number");
    try {
        server_config->port_ = safe_atoi(port);
    } catch (const std::overflow_error &e) {
        throw InvalidConfigException(current_line_, "port is overflow");
    } catch (const std::runtime_error &e) {
        throw InvalidConfigException(current_line_, "port must be positve number");
    }
}

void ConfigParser::parse_listen(ServerConfig *server_config) {
    Consume("listen");
    parse_port(server_config);
    Consume(";");
}

void ConfigParser::parse_server_name(ServerConfig *server_config) {
    server_config->server_name_ = ConsumeToken();
}

void ConfigParser::parse_server_name_directive(ServerConfig *server_config) {
    Consume("server_name");
    parse_server_name(server_config);
    Consume(";");
}

void ConfigParser::parse_server_block(std::map<ServerConfigKey, ServerConfig>* config) {
    ServerConfig server_config;
    Consume("server");
    Consume("{");
    parse_listen(&server_config);
    parse_server_name_directive(&server_config);
    parse_location_directives(&server_config);
    Consume("}");

    ServerConfigKey key(server_config.port_, server_config.server_name_);
    if (config->find(key) != config->end())
        throw InvalidConfigException(current_line_,"server_name or port duplicate");
    config->insert(std::make_pair(key, server_config));
}

void ConfigParser::parse_config(std::map<ServerConfigKey, ServerConfig>* config) {
    while (!is_end()) {
        parse_server_block(config);
    }
}

std::map<ServerConfigKey, ServerConfig> ConfigParser::Parse() {
    std::map<ServerConfigKey, ServerConfig> config;

    parse_config(&config);
    return config;
}

InvalidConfigException::InvalidConfigException(size_t line, std::string msg) throw(): line_(line), msg_(msg) {}

InvalidConfigException::~InvalidConfigException() throw() {}

const char* InvalidConfigException::what() const throw() {
    std::stringstream ss;
    ss << "line: " << line_ << " " << msg_;
    return ss.str().c_str();
}

int main() {
    ConfigParser config_parser("./srcs/config/server.conf");
    std::map<ServerConfigKey, ServerConfig> conf = config_parser.Parse();

    std::map<ServerConfigKey, ServerConfig>::iterator it;
    for (it = conf.begin(); it != conf.end(); it++) {
        std::cerr << it->second.ToString() << std::endl;
    }
}
