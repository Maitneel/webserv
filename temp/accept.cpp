#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <poll.h>
#include <iostream>
#include <vector>
using namespace std;

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

int main() {
    struct addrinfo hint, *res, *ai;
    bzero(&hint, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, "8080", &hint, &res);
    int socket_fd;

    ai = res;
    for (ai = res; ai != NULL; ai = ai->ai_next) {
        socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }
        int sockopt_arg = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_arg, sizeof(int))) {
            std::cerr << "setsockopt: failed set SO_REUSEADDR option" << std::endl;
        }
        struct sockaddr socket_addr;
        bzero(&socket_addr, sizeof(socket_addr));
        socket_addr.sa_family = AF_INET;
        if (bind(socket_fd, ai->ai_addr, ai->ai_addrlen) != 0) {
            close(socket_fd);
            continue;
        }
        if (listen(socket_fd, 10) != 0) {
            close(socket_fd);
            continue;
        }
    }

    vector<pollfd> pfd;
    struct pollfd npfd = {socket_fd, POLLIN, 0};
    pfd.push_back(npfd);

    for (size_t loop_count = 0; loop_count < 10; loop_count++) {
        poll(pfd.data(), pfd.size(), -1);
        cerr << "===============================" << endl;
        for (size_t i = 0; i < pfd.size(); i++) {
            cerr << pfd[i].fd << ": " << pfd[i].revents << endl;
        }
        cerr << "===============================" << endl;
        
        for (size_t i = 0; i < pfd.size(); i++) {
            if ((pfd[i].revents & POLLIN) == POLLIN) {
                if (pfd[i].fd == socket_fd) {
                    int conneciton_fd = accept(socket_fd, NULL, NULL);
                    struct pollfd npfd = {conneciton_fd, POLLIN, 0};
                    pfd.push_back(npfd);
                } else {
                    size_t buf_size = 50;
                    char buf[buf_size];
                    int recv_ret;
                    // do
                    // {
                        recv_ret = recv(pfd[i].fd, buf, buf_size, 0);
                        cerr << string(buf, recv_ret);
                    // } while (0 < recv_ret);
                    cerr << endl;
                }
            }
        }
    }

}
