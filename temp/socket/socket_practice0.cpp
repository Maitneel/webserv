#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <poll.h>
using namespace std;

#define debug(s) std::cout << #s << ' ' << s << std::endl; 

const char *port = "2525";



void print_addrinfo(struct addrinfo data) {
    debug(data.ai_flags);
    debug(data.ai_family);
    debug(data.ai_socktype);
    debug(data.ai_protocol);
    debug(data.ai_addrlen);
    debug(data.ai_addr);
    // debug(data.ai_canonname);
    // debug(data.ai_next);
}

void print_pollfd(struct pollfd fd) {
    debug(fd.fd);
    debug(fd.events);
    debug(fd.revents);
}

int sfd = -1;
struct addrinfo *result;

void sig_recive(int n) {

    
    close(sfd);
    for (size_t i = 0; i < 1024; i++) {
        close(i);
    }
    
    freeaddrinfo(result);
    sleep(20);
    exit(0);
}

int main() {
    signal(SIGINT, sig_recive);


    struct addrinfo hint;

    bzero(&hint, sizeof(struct addrinfo));

    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hint, &result);
    print_addrinfo(*result);

    sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    // struct linger lin;
    // bzero(&lin, sizeof(struct linger));
    // lin.l_linger = 1;
    // lin.l_onoff = 0;
    // if (setsockopt(sfd, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger))) {
    //     perror("setsocketopt");
    // }
    int n = 0;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));

    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    if (bind(sfd, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind");
        return 1;
    }

    if (listen(sfd, SOMAXCONN) != 0) {
        perror("listen");
        debug(errno);
        return 0;
    }

    size_t buffer_size = 1024;
    char buffer[buffer_size];
    while (1) {
        cerr << "listening..." << endl;
        size_t poll_fd_size = 2;
        pollfd poll_fd[poll_fd_size];
        poll_fd[0].events = POLL_IN;
        poll_fd[1].events = POLLPRI;

        poll(poll_fd, poll_fd_size, -1);
        int connect_fd = accept(sfd, NULL, NULL);
        cerr << "recive" << endl;
        recv(connect_fd, buffer, buffer_size, 0);

        std::string response;
        response += "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "\r\n";
        send(connect_fd, response.c_str(), response.length(), 0);
        close(connect_fd);
    }



    // size_t pfd_size = 10;
    // struct pollfd pfd[pfd_size];
    // bzero(pfd, sizeof(struct pollfd) * pfd_size);
    // pfd[0].events = POLLIN;
    // pfd[1].events = POLL_IN;
    // pfd[2].events = POLLIN;
    // pfd[3].events = POLLIN;
    // pfd[4].events = POLLIN;
    // // pfd[0].events = POLLERR;
    // // pfd[1].events = POLLHUP;
    // // pfd[2].events = POLLIN;
    // // pfd[3].events = POLLNVAL;
    // // pfd[4].events = POLLOUT;
    // // pfd[5].events = POLLPRI;
    // // pfd[6].events = POLLRDBAND;
    // // pfd[7].events = POLLRDNORM;
    // // pfd[8].events = POLLWRBAND;
    // cerr << "poll calling ..." << endl;;
    // poll(pfd, pfd_size, -1);
    // for (size_t i = 0; i < pfd_size; i++) {
    //     print_pollfd(pfd[i]);
    // }
    




}