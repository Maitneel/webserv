#include <unistd.h>

#include <iostream>
#include <stdexcept>

#include "proccessing.hpp"
#include "defines.hpp"
#include "gen_html.hpp"
using namespace std;

int main() {
    
    std::string method = getenv("REQUEST_METHOD");
    // char *cookie_cstr = getenv("HTTP_COOKIE");
    // std::map<std::string, std::string> cookie = make_cookie_map();

    if (access(INDEX_HTML_PATH_CSTR, F_OK)) {
        init_index_html();
    }
    try {
        cerr << "cgi method " << method << "------------------------------------ " <<endl;
        if (method == "GET") {
            get_method();
        } else if (method == "POST") {
            post_method();
        } else if (method == "DELETE") {
            std::cerr << "cgi delete --------------------------------------------------" << std::endl;
            delete_method();
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}