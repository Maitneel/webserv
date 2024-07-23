#pragma once

#include <vector>
#include "config.hpp"

#define MAX_BACKLOG 5
#define BUFFER_SIZE 1024

class Socket {
public:
    int          socket_fd;
    ServerConfig config;
    Socket(int socket_fd, ServerConfig config);
};

class Server {
private:
    std::vector<Socket> sockets;
public:
    Server(std::vector<ServerConfig> confs);
    void eventLoop();
};
