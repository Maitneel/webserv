#include <stdexcept>
#include <vector>

#include "poll_selector.hpp"

PollSelector::PollSelector() {}

PollSelector::~PollSelector() {}

static short ConvertPollEvent(SelectorEvent event) {
    switch (event) {
        case kEventRead:
            return POLLIN;
        case kEventWrite:
            return POLLOUT;
        case kEvnetError:
            return POLLERR | POLLHUP | POLLNVAL;
        default:
            return 0;
    }
}

static SelectorEvent ConvertSelectorEvent(short event) {
    switch (event) {
        case POLLIN:
            return kEventRead;
        case POLLOUT:
            return kEventWrite;
        case POLLERR | POLLHUP | POLLNVAL:
            return kEvnetError;
        default:
            return kEvnetError;
    }
}

void PollSelector::Register(int fd, SelectorEvent event) {
    struct pollfd poll_fd = {fd, ConvertPollEvent(event), 0};

    std::vector<pollfd>::iterator it;
    for (it = this->fds_.begin(); it != this->fds_.end(); it++) {
        if (it->fd == fd) {
            it->events = poll_fd.events;
            it->revents = 0;
            return;
        }
    }
    this->fds_.push_back(poll_fd);
}

void PollSelector::Unregister(int fd) {
    std::vector<pollfd>::iterator it;
    for (it = this->fds_.begin(); it != this->fds_.end(); it++) {
        if (it->fd == fd) {
            this->fds_.erase(it);
            return;
        }
    }
}

std::vector<FDEvent> PollSelector::Select(int timeout) {
    std::vector<FDEvent> events;
    int ret = poll(this->fds_.data(), this->fds_.size(), timeout);
    if (ret < 0)
        throw std::runtime_error("poll: failed");
    if (ret == 0)
        return events;
    std::vector<pollfd>::iterator it;
    for (it = this->fds_.begin(); it != this->fds_.end(); it++) {
        if (it->revents != 0) {
            FDEvent event = {it->fd, ConvertSelectorEvent(it->revents)};
            events.push_back(event);
        }
        it->revents = 0;
    }
    return events;
}
