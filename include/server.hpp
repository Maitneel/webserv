#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include "config.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "http_context.hpp"
#include "poll_selector.hpp"
#include "event_dispatcher.hpp"

#define MAX_BACKLOG 5
#define BUFFER_SIZE 1024

class Socket {
 private:
    int          socket_fd;
    ServerConfig config;
 public :
    Socket(int socket_fd, ServerConfig config);
    ~Socket();
    int   GetSocketFd();
    const ServerConfig& GetConfig();
};

class Server {
 private:
    ServerEventDispatcher dispatcher_;
    std::vector<Socket> sockets_;
    std::map<int, HTTPContext> ctxs_;

    void routing(const int &connection_fd, const int &socket_fd);

 public:
    explicit Server(std::vector<ServerConfig> confs);
    ~Server();
    ServerConfig GetConfigByFd(int fd);
    HTTPResponse GetHandler(int fd, const HTTPRequest& req);
    void EventLoop();
    bool IsIncludeFd(int fd);
    void AppendBuffer(std::string str);
};

#endif  // INCLUDE_SERVER_HPP_
