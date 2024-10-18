#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

#include <unistd.h>
#include <vector>
#include "config.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

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
    std::vector<Socket> sockets_;
 public :
    explicit Server(std::vector<ServerConfig> confs);
    ~Server();
    ServerConfig GetConfigByFd(int fd);
    HTTPResponse GetHandler(int fd, const HTTPRequest& req);
    void EventLoop();
    bool IsIncludeFd(int fd);
};

class HTTPContext {
public:
   int socket_fd_;
   std::string buffer_;
   pid_t cgi_pid_;

};

#endif  // INCLUDE_SERVER_HPP_
