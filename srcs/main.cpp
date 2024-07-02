#include <iostream>

#include "hoge.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <string>
#include <iostream>

#define MAX_BACKLOG 5
#define BUFFER_SIZE 1024

int listen_socket(const char *port)
{
    struct addrinfo hints, *res, *ai;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        // TODO: error handling
        std::exit(1);
    }

    for (ai = res; ai; ai=ai->ai_next) {
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0) {
            close(sock);
            continue;
        }
        if (bind(sock, res->ai_addr, res->ai_addrlen) < 0)
        {
            close(sock);
            continue;
        }
        if (listen(sock, MAX_BACKLOG) < 0)
        {
            close(sock);
            continue;
        }
        freeaddrinfo(res);
        return sock;
    }
    return -1;
}

std::string read_request(int fd)
{
    char buf[BUFFER_SIZE];

    int n_read = recv(fd, buf, BUFFER_SIZE-1, 0);
    if (n_read <= 0)
    {
        // TODO: error handling
        std::exit(1);
    }

    return std::string(buf, n_read);
}

void response_to_client(int fd, std::string content)
{
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
    return ;
}

void server_main(int server_fd)
{
    while(true)
    {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof addr;

        int sock = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if (sock < 0){
            // TODO: error handling
            std::exit(1);
        }
        // TODO: non blocking...
        std::string request_content = read_request(sock);
        response_to_client(sock, request_content);
        close(sock);
    }
}

int main() {
    int server_fd;
    const char* port = "8080";

    server_fd = listen_socket(port);
    server_main(server_fd);
    close(server_fd);
    return 0;
}
