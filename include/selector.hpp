#pragma once

#include <vector>

enum SelectorEvent {
    kEventRead,
    kEventWrite,
    kEvnetError
};

struct FDEvent {
    int fd;
    SelectorEvent event;
};

class Selector {
 public:
    virtual void Register(int fd, SelectorEvent event) = 0;
    virtual void Unregister(int fd) = 0;
    virtual std::vector<FDEvent> Select(int timeout) = 0;
};
