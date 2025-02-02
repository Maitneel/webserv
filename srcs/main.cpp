#include <iostream>
#include <map>
#include <string>
#include "server.hpp"
#include "config.hpp"
#include "http_request.hpp"
#include "event_dispatcher.hpp"
#include "config_parser.hpp"

std::map<ServerConfigKey, ServerConfig> hard_coding_config();

// /*
int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "usage: ./webserve ${CONFIG_FILE_PATH}" << std::endl;
        return 1;
    }
    try {
        ConfigParser config_parser(argv[1]);
        std::map<ServerConfigKey, ServerConfig> conf = config_parser.Parse();

        for (std::map<ServerConfigKey, ServerConfig>::iterator it = conf.begin(); it != conf.end(); it++) {
            std::cerr << it->second.ToString() << std::endl;
        }

        Server server(conf);

        server.EventLoop();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }

    return 0;
}
// */
