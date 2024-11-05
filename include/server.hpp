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
    PollSelector selector_;
    std::vector<Socket> sockets_;
    std::map<int, HTTPContext> ctxs_;
    void AcceptRequest(int fd);
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
