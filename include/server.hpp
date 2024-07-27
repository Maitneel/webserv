#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

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
    int   getSocketFd();
    const ServerConfig& getConfig();
};

class Server {
 private:
    std::vector<Socket> sockets;
 public :
    explicit Server(std::vector<ServerConfig> confs);
    ~Server();
    ServerConfig getConfigByFd(int fd);
    HTTPResponse getHandler(int fd, const HTTPRequest& req);
    void eventLoop();
};

#endif  // INCLUDE_SERVER_HPP_
