#include <sys/socket.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <iostream>
using namespace std;

#define debug(s) std::cerr << #s << '\'' << (s) << "' " << strerror(errno) << endl;

void child(int fd) {
    usleep(1000);
    char *buf = strdup("hoge");
    cerr << "writing fd: " << fd << endl;
    errno = 0;
    debug((write(fd, buf, 4)));
    usleep(1000);
    close(fd);
    free(buf);
    exit(0);
}

int main() {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) {
        cerr << "fail create socket" << endl;
        return 1;
    }

    fcntl(sv[0], F_SETFL, O_NONBLOCK);
    fcntl(sv[1], F_SETFL, O_NONBLOCK);

    pid_t pid = fork();
    int pairent_fd = sv[0];
    int child_fd = sv[1];

    if (pid == 0) {
        write(pairent_fd, "hoge", 4);
        close(pairent_fd);
        child(child_fd);
    }
    close(child_fd);
    char temp[10];
    cerr << "reading_fd: " << pairent_fd << endl;
    for (int i = 0; i < 10; i++) {
        errno = 0;
        debug(read(pairent_fd, temp, 1))
        errno = 0;
        int ret;
        debug(waitpid(pid, &ret, WNOHANG));
        usleep(1000);
    }
    close(sv[0]);
}
