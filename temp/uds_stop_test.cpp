#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
using namespace std;

int main() {
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);

    long long writed = 0;
    while (true) {
        writed += write(sv[0], "0123456789", 10);

        // if (writed % 10000 == 0) {
            cerr << writed << endl;
        // }
    }
}