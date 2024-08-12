#include <stdexcept>

#include "poll_wraper.hpp"

PollWraper::PollWraper() {}

PollWraper::~PollWraper() {}

void PollWraper::Register(int fd, short event_mask) {
    std::vector<pollfd>::iterator it;
    for (it = this->fds_.begin(); it != this->fds_.end(); it++) {
        if (it->fd == fd) {
            it->events = event_mask;
            it->revents = 0;
            return;
        }
    }
    struct pollfd poll_fd = {fd, event_mask, 0};
    this->fds_.push_back(poll_fd);
}

void PollWraper::Unregister(int fd) {
    std::vector<pollfd>::iterator it;
    for (it = this->fds_.begin(); it != this->fds_.end(); it++) {
        if (it->fd == fd) {
            this->fds_.erase(it);
            return;
        }
    }
}

std::vector<PollEvent> PollWraper::Poll(int timeout) {
    std::vector<PollEvent> events;
    int ret = poll(this->fds_.data(), this->fds_.size(), timeout);
    if (ret < 0)
        throw std::runtime_error("poll: failed");
    if (ret == 0)
        return events;
    std::vector<pollfd>::iterator it;
    for (it = this->fds_.begin(); it != this->fds_.end(); it++) {
        if (it->revents != 0) {
            struct PollEvent poll_event = {it->fd, it->revents};
            events.push_back(poll_event);
        }
        it->revents = 0;
    }
    return events;
}
