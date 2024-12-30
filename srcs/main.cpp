#include <iostream>
#include <map>
#include <string>
#include "server.hpp"
#include "config.hpp"
#include "http_request.hpp"
#include "event_dispatcher.hpp"

std::multimap<std::string, ServerConfig> hard_coding_config();

// /*
int main() {
    std::map<ServerConfigKey, ServerConfig> server_confs = parse_config("");

    Server server(server_confs);

    server.EventLoop();

    return 0;
}
// */
