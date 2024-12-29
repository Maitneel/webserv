#pragma once

#include <poll.h>
#include <vector>
#include "selector.hpp"

SelectorEvent ConvertSelectorEvent(short event);
short ConvertPollEvent(SelectorEvent event);

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
