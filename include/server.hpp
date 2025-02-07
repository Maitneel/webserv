#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "config.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "http_context.hpp"
#include "poll_selector.hpp"
#include "event_dispatcher.hpp"

#define MAX_BACKLOG 20000
#define BUFFER_SIZE 1024

class SocketList {
 public:
    SocketList();
    ~SocketList();

    void AddSocket(const int &port, const int &fd);
    int GetPort(const int &fd);
    int GetFd(const int &port);

 private:
    std::map<int, int> port_fd_pair_;
    std::map<int, int> fd_port_pair_;
};

class Server {
 private:
    ServerEventDispatcher dispatcher_;
    SocketList socket_list_;
    std::map<ServerConfigKey, ServerConfig> config_;
    std::map<int, HTTPContext> ctxs_;
    std::set<pid_t> pid_killed_by_webserve_;

    const LocatoinConfig &GetLocationConfig(const int &port, const HTTPRequest &req);

    void RoutingByLocationConfig(HTTPContext *ctx, const ServerConfig &server_config, const LocatoinConfig &loc_conf, const std::string &req_uri, const int &connection_fd);
    void routing(const int &connection_fd, const int &socket_fd);
    void CallCGI(const int &connection_fd, HTTPRequest *req, const std::string &cgi_path, const std::string &loc_name);
    void HandlingChildPID();
    void SendresponseFromCGIresponse(const int &connection_fd, const std::string &cgi_response_string);
    void SendresponseFromFile(const int &connection_fd, const std::string &file_content, const std::string &content_type);
    void SendErrorResponce(const int &stat, const ServerConfig config, const int &connection_fd);
    void CloseConnection(const int connection_fd);

    const ServerConfig &GetConfig(const int &port, const std::string &host_name);

 public:
    explicit Server(std::map<ServerConfigKey, ServerConfig> confs);
    ~Server();
    // ServerConfig GetConfigByFd(int fd);
    // TODO(everyone): 関数の思考を変えたので関数名が適切か検討する //
    void GetMethodHandler(HTTPContext *context, const std::string &req_path, const ServerConfig &server_config, const LocatoinConfig &location_config);
    void HeadMethodHandler(HTTPContext *context, const std::string &req_path, const ServerConfig &server_config, const LocatoinConfig &location_config);
    void EventLoop();
};

std::string generate_autoindex_file(const std::string &dir_name, const std::string &req_path);

#endif  // INCLUDE_SERVER_HPP_
