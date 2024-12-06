#include <poll.h>
#include <unistd.h>
#include <iostream>
using namespace std;

int main() {
    struct pollfd pfd[1];

    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
    pfd[0].fd = STDIN_FILENO;

    char buf[1024];
    buf[0] = 0;

    for (size_t i = 0; i < 5; i++) {
        int pollret = poll(pfd, 1, -1);
        if (pfd[0].revents | POLLIN) {
            // buf[read(STDIN_FILENO, buf, 1024)] = 0;
            
        }
        cerr << pfd[0].fd << ' ' <<  pfd[0].events << ' ' << pfd[0].revents << ' ' << buf <<  endl;
    }
}