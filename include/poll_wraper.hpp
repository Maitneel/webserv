#pragma once

#include <poll.h>

#include <vector>

struct PollEvent {
    int fd;
    int event;
};

class PollWraper {
 private:
    std::vector<pollfd> fds_;
 public:
    PollWraper();
    ~PollWraper();
    void Register(int fd, short event_mask);
    void Unregister(int fd);
    std::vector<PollEvent> Poll(int timeout);
};
