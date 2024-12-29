#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <iostream>
using namespace std;

int main(int argc, char **argv) {
    pollfd pfd[1];
    pfd[0].fd = open(argv[1], (O_RDONLY | O_NONBLOCK));
    pfd[0].revents = 0;
    pfd[0].events = POLLIN;
    do {
        poll(pfd, 1, 1000);
        cout << pfd[0].fd << ' ' << pfd[0].revents << ' ' << pfd[0].events << endl;
        char buf[1024];
        int read_ret = read(pfd[0].fd, buf, 1023);
        cerr << buf;
    } while (pfd[0].fd);
    cout << pfd[0].fd << ' ' << pfd[0].revents << ' ' << pfd[0].events << endl;
}