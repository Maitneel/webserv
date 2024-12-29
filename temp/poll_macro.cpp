#include <poll.h>
#include <iostream>
using namespace std;

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

int main() {


    debug(POLLERR)
    debug(POLLHUP)
    debug(POLLIN)
    debug(POLLNVAL)
    debug(POLLOUT)
    debug(POLLPRI)
    debug(POLLRDBAND)
    debug(POLLRDNORM)
    debug(POLLWRBAND)
    debug(POLLWRNORM)

}