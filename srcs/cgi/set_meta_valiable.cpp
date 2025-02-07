#include <vector>
#include <map>
#include <string>
#include <cstring>

#include "http_request.hpp"
#include "cgi_valid.hpp"
#include "extend_stdlib.hpp"

#define LAST_STRING ""

// ここlint通らないけど、このまま行きたい気がする //
const std::string defined_meta_val_by_rfc_array[] = {
    "auth-scheme",
    "content-type",
    "content-length",
    "host",
    LAST_STRING
};

std::string make_env_format(const std::string &s, const std::string & t)  {
    return (s + "=" + t);
}

static std::string join_header(const std::vector<std::string> &v) {
    std::string joined;
    if (v.size() == 0) {
        return joined;
    }
    joined = v[0];
    for (size_t i = 1; i < v.size(); i++) {
        joined += ',';
        joined += v[i];
    }
    return joined;
}

static void add_env(const std::string &env_name, const std::string &env_key, const std::map<std::string, std::vector<std::string> > &header_map, bool(*valid_func)(const std::string &), std::vector<std::string> *env) {
    if (header_map.find(env_key) == header_map.end()) {
        return;
    }
    std::string value = join_header(header_map.find(env_key)->second);
    if (!rfc_cgi::is_meta_valiable_value(value)) {
        // 正しくない場合どうする？ //
        return;
    }
    if (valid_func != NULL && !valid_func(value)) {
        return;
    }
    env->push_back(make_env_format(env_name, value));
}

static void add_auth_type_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    add_env("AUTH_TYPE", "auth-scheme", req.header_, NULL, env);
}

static void add_content_length_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    size_t conten_length = req.entity_body_.length();
    // if (conten_length == 0) {
    //     return;
    // }
    env->push_back(make_env_format("CONTENT_LENGTH", size_t_to_string(conten_length)));
}

static void add_content_type_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    add_env("CONTENT_TYPE", "content-type", req.header_, NULL, env);
}

static void add_gateway_interface_to_env(std::vector<std::string> *env) {
    env->push_back("GATEWAY_INTERFACE=CGI/1.1");
}

static void add_path_info_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    // TODO(maitneel): 実装しないかも //
    (void)(req);
    (void)(env);
}

static void add_query_string_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    const std::string::size_type question_index = req.get_request_uri().find("?");
    if (question_index == std::string::npos) {
        env->push_back(make_env_format("QUERY_STRING", ""));
        return;
    }
    std::string query_string = req.get_request_uri().substr(question_index);
    // TODO(maitneel): valid query_string;
    if (!rfc_cgi::is_meta_valiable_value(query_string)) {
        env->push_back(make_env_format("QUERY_STRING", ""));
        return;
    }
    env->push_back(make_env_format("QUERY_STRING", query_string));
}

static void add_remote_addr_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    // TODO(maitneel): client の IP address を入れるがわからないので後でする //
    (void)(req);
    (void)(env);
}

// REMOTE_HOST は~めんどくさいから~わからないからやらないでいいかなー //

// REMOTE_USER MUST　って書いてあったけど今のところいかなる認証もサポートしてないから作んなくて良くない？ //

static void add_method_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    // valid してエラーになっても困るししなくていいよね? //
    env->push_back(make_env_format("REQUEST_METHOD", req.get_method()));
}

static void add_script_name_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    // TODO(maitneel): なんかMUSTって書いてあるけどよくわかんないからあとで　　//
    (void)(req);
    (void)(env);
}

static void add_server_name_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    add_env("SERVER_NAME", "host", req.header_, NULL, env);
}

static void add_server_port_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    // TODO(maitneel): port 持てるようにしないと実装できねぇ〜(MUST) //
    (void)(req);
    (void)(env);
}


static void add_server_protocol_to_env(const HTTPRequest &req, std::vector<std::string> *env) {
    // ここもvalidしないぞ //
    env->push_back(make_env_format("SERVER_PROTOCOL", req.get_protocol()));
}

// SERVER_SOFTWARE よくわかんない //


bool is_include(const std::string &target, const std::string list[]) {
    for (size_t i = 0; list[i] != LAST_STRING; i++) {
        if (target == list[i]) {
            return true;
        }
    }
    return false;
}

static void add_other_env(HTTPRequest request, std::vector<std::string> *env) {
    for (std::map<std::string, std::vector<std::string> >::iterator it = request.header_.begin(); it != request.header_.end(); it++) {
        if (!is_include(it->first, defined_meta_val_by_rfc_array)) {
            std::string key = "HTTP_" + it->first;
            to_upper(&key);
            env->push_back(make_env_format(key, join_header(it->second)));
        }
    }
}


static void add_env_from_request(const HTTPRequest &request, std::vector<std::string> *env) {
    add_auth_type_to_env(request, env);
    add_content_length_to_env(request, env);
    add_content_type_to_env(request, env);
    add_gateway_interface_to_env(env);
    add_path_info_to_env(request, env);
    add_query_string_to_env(request, env);
    add_remote_addr_to_env(request, env);
    add_method_to_env(request, env);
    add_script_name_to_env(request, env);
    add_server_name_to_env(request, env);
    add_server_port_to_env(request, env);
    add_server_protocol_to_env(request, env);

    add_other_env(request, env);
}

char **make_env_array(const HTTPRequest &request, const std::string &path_info) {
    std::vector<std::string> env_vector;
    add_env_from_request(request, &env_vector);
    env_vector.push_back("PATH_INFO=" + path_info);

    char **env = new char*[env_vector.size() + 1];
    for (size_t i = 0; i < env_vector.size(); i++) {
        env[i] = strdup(env_vector.at(i).c_str());
    }
    env[env_vector.size()] = NULL;
    return env;
}

/*
#include <iostream>
int main(int argc, char **argv) {
    // if (argc != 2) {
    //     return 0;
    // }
    std::string req_str;
    req_str += "GET / HTTP/1.1\r\n";
    req_str += "Host: localhost:8080\r\n";
    req_str += "User-Agent: curl/7.79.1\r\n";
    req_str += "auth-scheme:basic\r\n";
    req_str += "\r\n";
    HTTPRequest req(req_str);
    req.print_info();
    std::vector<std::string> env;
    set_auth_type(req, &env);
    for (size_t i = 0; i < env.size(); i++) {
        std::cout << env[i] << std::endl;
    }
}
// */
