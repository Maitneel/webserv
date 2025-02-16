#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "config.hpp"

class ConfigParser {
 private:
    size_t read_index_;
    size_t current_line_;
    std::string raw_str_;
    const std::string tokens_;

    std::string GetToken();
    std::string ConsumeToken();
    void Consume(const std::string& expect);

    void ThrowInvalidException(const std::string& msg);
    bool is_config_token(char c);
    void valid_method(const std::string& str);
    void valid_status_code(const std::string& str);
    void valid_location_path(const std::string& route);
    void valid_error_page_path(const std::string& path);
    void valid_url(const std::string& url);
    void valid_path(const std::string& path);
    void valid_cgi_path(const std::string& path);
    void parse_url(LocationConfig *location_config);
    void parse_return_directive(LocationConfig *location_config);
    void parse_cgi_path(LocationConfig *location_config);
    void parse_max_body_size(LocationConfig *location_config);
    void parse_max_body_size_directive(LocationConfig *location_config);
    void parse_autoindex_directive(LocationConfig *location_config);
    void parse_index_files(LocationConfig *location_config);
    void parse_index_directive(LocationConfig *location_config);
    void parse_location_path(LocationConfig *location_config);
    void parse_path(LocationConfig *location_config);
    void parse_root_directive(LocationConfig *location_config);
    void parse_method(LocationConfig *location_config);
    void parse_method_directive(LocationConfig *location_config);
    void parse_location_directive(ServerConfig *server_config);
    void parse_location_directives(ServerConfig *server_config);
    void parse_port(ServerConfig *sever_config);
    void parse_listen(ServerConfig *sever_config);
    void parse_server_name(ServerConfig *server_config);
    void parse_server_name_directive(ServerConfig *server_config);
    void parse_error_page(ServerConfig *server_config);
    void parse_server_block(std::map<ServerConfigKey, ServerConfig>* config);
    void parse_config(std::map<ServerConfigKey, ServerConfig>* config);
    bool is_end();

 public:
    explicit ConfigParser(const std::string &file_path);
    std::map<ServerConfigKey, ServerConfig> Parse();
};

class InvalidConfigException: public std::exception {
 private:
    size_t line_;
    std::string msg_;
 public:
    InvalidConfigException(size_t line, std::string msg) throw ();
    ~InvalidConfigException() throw();
    const char* what() const throw();
    std::string GetInvalidLineMessage();
};
