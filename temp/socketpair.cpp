#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
using namespace std;

void pairent(int fd) {
    char buf[10];
    write(fd, "fp_sv0", 6);
    buf[read(fd, buf, 10)] = 0;
    cout << "pairent: " << buf << endl;
}

void child(int fd) {
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    system("./child.out");
}


int main() {
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    cout << "socket_pair: " << sv[0] << ' ' << sv[1] << endl;
    pid_t pid = fork();

    if (pid == 0) {
        close(sv[0]);
        child(sv[1]);
    } else {
        close(sv[1]);
        pairent(sv[0]);
    }
    return 0;
}
