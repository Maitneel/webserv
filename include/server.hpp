#ifndef _INCLUDE_HTTP_VALIDATION_HPP_
#define _INCLUDE_HTTP_VALIDATION_HPP_

#include <vector>
#include "config.hpp"

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
    void eventLoop();
};

#endif
