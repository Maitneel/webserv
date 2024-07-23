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
#include "HTTPRequest.hpp"


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

void response_to_client(int fd, std::string content) {
    std::string response;
    std::ostringstream oss;

    oss << content.size() + 2;
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: ";
    response += oss.str() + "\r\n";
    response += "\r\n";
    response += content;
    response += "\r\n";

    send(fd, response.c_str(), response.length(), 0);
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

Socket::Socket(int socket_fd, ServerConfig config): socket_fd(socket_fd), config(config) {}


void Server::eventLoop() {
    // 一旦、最初のFDのみ
    int server_fd = this->sockets[0].socket_fd;
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
        std::cout << '[' << get_formated_date() << "] " << request.get_method() << ' ' << request.get_request_uri() << ' ' << request.get_protocol() << std::endl;

        response_to_client(sock, request_content);
        close(sock);

        // break させるようの処理 //
        std::string method = request.get_method();
        if (method == "kill" || method == "KILL" || method == "Kill") {
            break;
        }
    }
}
