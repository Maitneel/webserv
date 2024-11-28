#include <iostream>
#include <vector>
#include "server.hpp"
#include "config.hpp"
#include "http_request.hpp"
#include "event_dispatcher.hpp"

// /*
int main() {
    std::vector<ServerConfig> server_confs = parse_config("");

    Server server(server_confs);

    server.EventLoop();

    return 0;
}
// */
