#include <poll.h>
#include <unistd.h>
#include <iostream>

using namespace std;

int main() {
    struct pollfd pfd[1];

    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
    for (size_t i = 0; i < 10; i++) {
        pfd[0].revents = 0;
        poll(pfd, 1, 1000);
        cerr << pfd[0].fd << ", " << pfd[0].revents << std::endl;
        pfd[0].revents = 0;
        poll(pfd, 1, 0);
        cerr << pfd[0].fd << ", " << pfd[0].revents << std::endl;
        char buffer[1024];
        buffer[0] = 0;
        if (pfd[0].revents) {
            int read_ret = read(STDIN_FILENO, buffer, 1024);
            buffer[read_ret] = 0;
            cerr << string(buffer, read_ret) << endl;
        }
    }
    
    

}