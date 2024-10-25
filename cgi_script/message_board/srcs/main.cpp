#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <map>

#include "proccessing.hpp"
#include "defines.hpp"
#include "gen_html.hpp"
#include "cookie.hpp"


using namespace std;

int main() {
    
    std::string method = getenv("REQUEST_METHOD");
    std::multimap<std::string, std::string> cookie = parse_cookie();

    // char *cookie_cstr = getenv("HTTP_COOKIE");
    // std::map<std::string, std::string> cookie = make_cookie_map();

    if (access(INDEX_HTML_PATH_CSTR, F_OK)) {
        init_index_html();
    }
    try {
        cerr << "cgi method " << method << "------------------------------------ " <<endl;
        if (method == "GET") {
            get_method(cookie);
        } else if (method == "POST") {
            post_method(cookie);
        } else if (method == "DELETE") {
            std::cerr << "cgi delete --------------------------------------------------" << std::endl;
            delete_method(cookie);
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}