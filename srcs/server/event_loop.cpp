#include <vector>
#include <map>

#include "poll_selector.hpp"

enum ServerEventType {
    kHTTPRequestHeader,
    kHTTPRequestBody,
    kHTTPResponse,
    kCGIRequest,
    kCGIResponse,
    kFILERead,
    kFIleWrite,
};

class ServerEvent {
    ServerEventType type_;
    HTTPContext;
};

class ServerEventSelector {
    PollSelector poll_selector_;
    std::map<int, HTTPContext>  contexts_;
    std::vector<int> socket_fds;  // constructor でもらう。//
};

std::vector<ServerEvent> ServerEventSelector::Select() {
    std::vector<FDEvent> events = poll_selector_.Select(100);
    std::vector<ServerEvent> server_events;

    std::vector<FDEvent>::const_iterator it;
    for (it = events.begin(); it != events.end(); it++) {
        if (this->IsIncludeFd(it->fd) && it->event == kEventRead) {
        } else if (it->event == kEventRead) {
    }
}
