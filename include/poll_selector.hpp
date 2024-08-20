#pragma once

#include <poll.h>
#include <vector>
#include "selector.hpp"

class PollSelector: public Selector {
 public:
    PollSelector();
    ~PollSelector();
    void Register(int fd, SelectorEvent event);
    void Unregister(int fd);
    std::vector<FDEvent> Select(int timeout);
 private:
    std::vector<pollfd> fds_;
    PollSelector(const PollSelector&);  // 明示的にコピーさせない
    PollSelector operator=(const PollSelector&);  // 明示的にコピーさせない
};
