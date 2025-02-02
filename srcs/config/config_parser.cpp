#include "config_parser.hpp"
#include "extend_stdlib.hpp"
#include <string>
#include <fstream>
#include <map>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <utility>

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
        throw InvalidConfigException(current_line_, read_index_, "expect " + expect + " but obtain " + token);
    }
}

void ConfigParser::parse_url(LocatoinConfig *location_config) {
    // TODO(taksaito) :  url のチェック
    std::string url = ConsumeToken();
    location_config->redirect_ = url;
}

void ConfigParser::parse_return_directive(LocatoinConfig *location_config) {
    Consume("return");
    parse_url(location_config);
    Consume(";");
}

void ConfigParser::parse_cgi_path(LocatoinConfig *location_config) {
    Consume("cgi_path");
    parse_path(location_config);
    Consume(";");
}

void ConfigParser::parse_number(LocatoinConfig *location_config) {
    std::string body_size_str = ConsumeToken();
    try {
        location_config->max_body_size_ = safe_atoi(body_size_str);
    } catch (const std::overflow_error &e) {
        // TODO(taksaito):  error handling
    } catch (const std::runtime_error &e) {
        // TODO(taksaito):  error handling
    }
}

void ConfigParser::parse_max_body_size_directive(LocatoinConfig *location_config) {
    Consume("max_body_size");
    std::string path = ConsumeToken();
    // TODO(taksaito) :  validation path;
    location_config->cgi_path_ = path;
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
        throw InvalidConfigException(current_line_, read_index_, "autoindex expect 'on' or 'off' but obtained " + token);
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
    // TODO(taksaito) :  pathのvalidation
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
    std::string path = ConsumeToken();
    // TODO(taksaito) :  pathのvalidation
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
        throw InvalidConfigException(current_line_, read_index_, "duplicate location route");
    server_config->location_configs_.insert(std::make_pair(location_config.name_, location_config));
}

void ConfigParser::parse_location_directives(ServerConfig *server_config) {
    do {
        parse_location_directive(server_config);
    } while(GetToken() == "location");
}

void ConfigParser::parse_port(ServerConfig *server_config) {
    std::string port = this->ConsumeToken();
    // TODO(taksaito): 数字かチェック
    try {
        server_config->port_ = safe_atoi(port);
    } catch (const std::overflow_error &e) {
        // TODO(taksaito):  error handling
    } catch (const std::runtime_error &e) {
        // TODO(taksaito):  error handling
    }
}

void ConfigParser::parse_listen(ServerConfig *server_config) {
    Consume("listen");
    parse_port(server_config);
    Consume(";");
}

void ConfigParser::parse_server_name(ServerConfig *server_config) {
    // TODO(takasaito): より厳密にチェック
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
        throw InvalidConfigException(current_line_, read_index_,"server_name or port duplicate");
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

InvalidConfigException::InvalidConfigException(size_t line, size_t read_index, std::string msg) throw(): line_(line), msg_(msg) {
    col_ = read_index % line;
}

InvalidConfigException::~InvalidConfigException() throw() {}

const char* InvalidConfigException::what() const throw() {
    std::stringstream ss;
    ss << "line: " << line_ << " " << "col: " << col_ << " " << msg_;
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
