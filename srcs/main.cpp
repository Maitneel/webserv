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
    if (2 < argc) {
        std::cerr << "usage: ./webserve ${CONFIG_FILE_PATH}" << std::endl;
        return 1;
    }
    try {
        std::string config_path;
        if (argc == 2) {
            config_path = argv[1];
        } else {
            config_path = std::string("./config/default.conf");
        }
        ConfigParser config_parser(config_path);
        std::map<ServerConfigKey, ServerConfig> conf = config_parser.Parse();

        for (std::map<ServerConfigKey, ServerConfig>::iterator it = conf.begin(); it != conf.end(); it++) {
            std::cerr << it->second.ToString() << std::endl;
        }

        Server server(conf);

        server.EventLoop();
    } catch (InvalidConfigException &e) {
        std::cerr << e.GetInvalidLineMessage() << std::endl;
        return 2;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 3;
    }

    return 0;
}
// */
