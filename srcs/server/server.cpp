#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <cstdlib>
#include <cstring>

#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "server.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

std::string int_to_str(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

std::string get_formated_date() {
    struct tm newtime;
    time_t ltime;
    char buf[50];

    ltime = time(&ltime);
    localtime_r(&ltime, &newtime);
    std::string now_string(asctime_r(&newtime, buf));
    now_string.erase(
        remove(now_string.begin(), now_string.end(), '\n'),
        now_string.end());
    return now_string;
}

std::string read_request(int fd) {
    char buf[BUFFER_SIZE];

    int n_read = recv(fd, buf, BUFFER_SIZE-1, 0);
    if (n_read <= 0) {
        // TODO(taksaito): error handling
        std::exit(1);
    }

    return std::string(buf, n_read);
}

void response_to_client(int fd, const HTTPResponse& response) {
    std::string response_raw = response.toString();
    if (send(fd, response_raw.c_str(), response_raw.length(), 0) != 0)
        close(fd);
    return;
}

int create_inet_socket(int port) {
    struct addrinfo hints, *res, *ai;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, int_to_str(port).c_str(), &hints, &res) != 0) {
        return -1;
    }

    for (ai = res; ai; ai=ai->ai_next) {
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0) {
            close(sock);
            continue;
        }
        // 本番環境として動かすならやらない方がいいと思うので、プリプロセッサで条件分岐した方がいいかもです //
        int sockopt_arg = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockopt_arg, sizeof(int))) {
            std::cerr << "setsockopt: failed set SO_REUSEADDR option" << std::endl;
        }
        if (bind(sock, res->ai_addr, res->ai_addrlen) < 0) {
            close(sock);
            continue;
        }
        if (listen(sock, MAX_BACKLOG) < 0) {
            close(sock);
            continue;
        }
        freeaddrinfo(res);
        return sock;
    }
    return -1;
}

Server::Server(std::vector<ServerConfig> confs) {
    std::vector<ServerConfig>::iterator it;
    for (it = confs.begin(); it != confs.end(); it++) {
        int sock = create_inet_socket(it->port);
        if (sock < 0)
            throw std::runtime_error("can not create tcp socket.");
        this->sockets.push_back(Socket(sock, *it));
    }
}

Server::~Server() {}

Socket::Socket(int socket_fd, ServerConfig config): socket_fd(socket_fd), config(config) {}

Socket::~Socket() {}

int Socket::getSocketFd() {
    return this->socket_fd;
}

const ServerConfig& Socket::getConfig() {
    return this->config;
}

void http_log(const HTTPRequest& request) {
    std::cout << '[' << get_formated_date() << "] " << request.get_method() << ' ' << request.get_request_uri() << ' ' << request.get_protocol() << std::endl;
    std::cout << "    header : {" << std::endl;
    for (std::map<std::string, std::string>::const_iterator i = request.header.begin(); i != request.header.end(); i++) {
        std::cout << "        " << i->first << ": " << i->second << std::endl;
    }
    std::cout << "    }" << std::endl;
    std::cout << "    body : {" << std::endl;
    std::cout << "        " << request.entity_body << std::endl;
    std::cout << "    }" << std::endl;
    return;
}

void Server::eventLoop() {
    // 一旦、最初のFDのみ
    int server_fd = this->sockets[0].getSocketFd();
    while(true) {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof addr;

        int sock = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if (sock < 0) {
            // TODO(taksaito): error handling
            std::exit(1);
        }
        // TODO(taksaito): non blocking...
        std::string request_content = read_request(sock);
        std::cerr << "resived " << std::endl;
        HTTPRequest request(request_content);
        http_log(request);

        HTTPResponse response(200, "text/html", request.entity_body);
        response_to_client(sock, response);
        close(sock);

        // break させるようの処理 //
        std::string method = request.get_method();
        if (method == "kill" || method == "KILL" || method == "Kill") {
            break;
        }
    }
}
